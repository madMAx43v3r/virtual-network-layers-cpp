/*
 * Object.cpp
 *
 *  Created on: Mar 7, 2016
 *      Author: mad
 */

#include <vnl/Object.h>
#include <vnl/Sample.h>
#include <vnl/Announce.hxx>
#include <vnl/LogMsg.hxx>
#include <vnl/Shutdown.hxx>
#include <vnl/Exit.hxx>


namespace vnl {

void Object::exit() {
	dorun = false;
}

StringWriter Object::log(int level) {
	StringOutput* out = 0;
	if(level <= log_level) {
		log_writer.level = level;
		out = &log_writer;
	}
	return StringWriter(out);
}

Timer* Object::set_timeout(int64_t micros, const std::function<void(Timer*)>& func, int type) {
	Timer* timer = memory.create<Timer>();
	timer->interval = micros;
	timer->func = func;
	timer->type = type;
	if(type != VNL_TIMER_MANUAL) {
		timer->reset();
	}
	timers.push(timer);
	return timer;
}

void Object::publish(Value* data, Address topic) {
	vnl::Sample* pkt = buffer.create<vnl::Sample>();
	pkt->src_addr = my_address;
	pkt->seq_num = next_seq++;
	pkt->data = data;
	send_async(pkt, topic);
}

bool Object::poll(int64_t micros) {
	int64_t to = micros;
	int64_t now = currentTimeMicros();
	for(Timer* timer : timers) {
		if(timer->active) {
			int64_t diff = timer->deadline - now;
			if(diff <= 0) {
				timer->func(timer);
				switch(timer->type) {
					case VNL_TIMER_REPEAT:
						timer->active = true;
						timer->deadline += timer->interval;
						break;
					case VNL_TIMER_MANUAL:
					case VNL_TIMER_ONCE:
						timer->active = false;
						break;
				}
				diff = timer->deadline - now;
				if(diff < 0) {
					diff = 0;
				}
			}
			if(diff < to || to == -1) {
				to = diff;
			}
		}
	}
	while(true) {
		Message* msg = stream.poll(to);
		if(!msg) {
			break;
		}
		if(!handle(msg)) {
			msg->ack();
		}
		to = 0;
	}
	return dorun;
}

bool Object::handle(Message* msg) {
	if(msg->msg_id == Packet::MID) {
		Packet* pkt = (Packet*)msg;
		return handle(pkt);
	}
	return false;
}

bool Object::handle(Packet* pkt) {
	uint32_t& last_seq = sources[pkt->src_addr];
	// TODO: handle sequence wrap around
	if(pkt->seq_num <= last_seq) {
		if(pkt->pkt_id == vnl::Frame::PID) {
			Frame* result = buffer.create<Frame>();
			send_async(result, pkt->src_addr);
		}
		pkt->ack();
		return true;
	}
	last_seq = pkt->seq_num;
	if(pkt->pkt_id == Sample::PID) {
		if(((Sample*)pkt->payload)->data->vni_hash() == vnl::Shutdown::VNI_HASH) {
			dorun = false;
		}
	} else if(pkt->pkt_id == Frame::PID) {
		Frame* request = (Frame*)pkt->payload;
		Frame* result = buffer.create<Frame>();
		result->seq_num = request->seq_num;
		buf_in.wrap(request->data, request->size);
		int size = 0;
		int id = in.getEntry(size);
		if(id == VNL_IO_INTERFACE) {
			uint32_t hash = 0;
			in.getHash(hash);
			while(!in.error()) {
				uint32_t hash = 0;
				int size = 0;
				int id = in.getEntry(size);
				if(id == VNL_IO_CALL) {
					in.getHash(hash);
					bool res = vni_call(in, hash, size);
					if(!res) {
						in.skip(id, size, hash);
						log(WARN).out << "VNL_IO_CALL failed: hash=" << vnl::hex(hash) << " size=" << size << vnl::endl;
					}
				} else if(id == VNL_IO_CONST_CALL) {
					in.getHash(hash);
					if(!vni_const_call(in, hash, size, out)) {
						in.skip(id, size, hash);
						out.putNull();
						log(WARN).out << "VNL_IO_CONST_CALL failed: hash=" << vnl::hex(hash) << " size=" << size << vnl::endl;
					}
					out.flush();
				} else if(id == VNL_IO_INTERFACE && size == VNL_IO_END) {
					break;
				} else {
					in.skip(id, size);
				}
			}
			result->size = buf_out.position();
			result->data = buf_out.release();
		}
		send_async(result, request->src_addr);
	}
	return false;
}

bool Object::sleep(int64_t secs) {
	return usleep(secs*1000*1000);
}

bool Object::usleep(int64_t micros) {
	int64_t now = currentTimeMicros();
	int64_t deadline = now + micros;
	while(now < deadline) {
		int64_t to = deadline - now;
		if(!poll(to)) {
			return false;
		}
		now = currentTimeMicros();
	}
	return true;
}

void Object::run() {
	while(dorun && poll(-1));
}

void Object::exec(Engine* engine_, Message* msg) {
	engine = engine_;
	stream.connect(engine_);
	open(my_address);
	Announce* announce = Announce::create();
	announce->instance.type = type_name();
	announce->instance.domain = my_domain;
	announce->instance.topic = my_topic;
	publish(announce, local_domain, "vnl/announce");
	main(engine_, msg);
	for(Address addr : ifconfig) {
		close(addr);
	}
	while(true) {
		Message* msg = stream.poll(0);
		if(msg) {
			msg->ack();
		} else {
			break;
		}
	}
	for(Timer* timer : timers) {
		timer->~Timer();
	}
	publish(Exit::create(), local_domain, "vnl/exit");
	stream.flush();
}

void Object::set_log_level(int32_t level) {
	log_level = level;
}

vnl::info::Class Object::get_class() const {
	vnl::info::Class res;
	res.name = type_name();
	// TODO
	return res;
}

Binary Object::vni_serialize() const {
	vnl::io::ByteBuffer buf;
	vnl::io::TypeOutput out(&buf);
	serialize(out);
	out.flush();
	Binary blob;
	blob.size = buf.position();
	blob.data = buf.release();
	return blob;
}

void Object::vni_deserialize(const vnl::Binary& blob) {
	vnl::io::ByteBuffer buf;
	vnl::io::TypeInput in(&buf);
	buf.wrap(blob.data, blob.size);
	vnl::read(in, *this);
}




}
