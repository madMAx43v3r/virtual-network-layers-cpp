/*
 * Object.cpp
 *
 *  Created on: Mar 7, 2016
 *      Author: mad
 */

#include <vnl/Object.h>
#include <vnl/Pipe.h>
#include <vnl/Sample.h>
#include <vnl/Announce.hxx>
#include <vnl/LogMsg.hxx>
#include <vnl/Topic.hxx>
#include <vnl/Shutdown.hxx>
#include <vnl/Exit.hxx>


namespace vnl {

void Object::serialize(vnl::io::TypeOutput& out) const {
	out.putNull();
}

void Object::exit() {
	dorun = false;
}

StringWriter Object::log(int level) {
	StringOutput* out = 0;
	if(level <= vnl_log_level) {
		log_writer.level = level;
		out = &log_writer;
	}
	return StringWriter(out);
}

Timer* Object::set_timeout(int64_t micros, const std::function<void()>& func, int type) {
	Timer& timer = timers.push_back();
	timer.interval = micros;
	timer.func = func;
	timer.type = type;
	if(type != VNL_TIMER_MANUAL) {
		timer.reset();
	}
	return &timer;
}

Object* Object::fork(Object* object) {
	engine->fork(object);
	return object;
}

Address Object::subscribe(const String& domain, const String& topic) {
	Topic* top = Topic::create();
	top->domain = domain;
	top->name = topic;
	publish(top, Address(local_domain, "vnl.topic"));
	return subscribe(Address(domain, topic));
}

Address Object::subscribe(Address address) {
	return stream.subscribe(address);
}

void Object::unsubscribe(Hash64 domain, Hash64 topic) {
	unsubscribe(Address(domain, topic));
}

void Object::unsubscribe(Address address) {
	stream.unsubscribe(address);
}

void Object::publish(Value* data, Hash64 domain, Hash64 topic) {
	publish(data, Address(domain, topic));
}

void Object::publish(Value* data, Address topic) {
	Sample* pkt = buffer.create<Sample>();
	pkt->data = data;
	send_async(pkt, topic);
}

void Object::send(Packet* packet, Address dst) {
	if(packet->src_addr.is_null()) {
		packet->src_addr = my_address;
	}
	stream.send(packet, dst);
}

void Object::send_async(Packet* packet, Address dst) {
	if(packet->src_addr.is_null()) {
		packet->src_addr = my_address;
	}
	stream.send_async(packet, dst);
}

void Object::send(Packet* packet, Basic* dst) {
	if(packet->src_addr.is_null()) {
		packet->src_addr = my_address;
	}
	stream.send(packet, dst);
}

void Object::send_async(Packet* packet, Basic* dst) {
	if(packet->src_addr.is_null()) {
		packet->src_addr = my_address;
	}
	stream.send_async(packet, dst);
}

void Object::send(Message* msg, Basic* dst) {
	stream.send(msg, dst);
}

void Object::send_async(Message* msg, Basic* dst) {
	stream.send_async(msg, dst);
}

void Object::flush() {
	stream.flush();
}

void Object::attach(Pipe* pipe) {
	pipes.push_back(pipe);
	pipe->ack(this);
}

void Object::close(Pipe* pipe) {
	pipes.remove(pipe);
	pipe->fin();
}

bool Object::poll(int64_t micros) {
	int64_t to = micros;
	int64_t now = currentTimeMicros();
	for(Timer& timer : timers) {
		if(timer.active) {
			int64_t diff = timer.deadline - now;
			if(diff <= 0) {
				switch(timer.type) {
					case VNL_TIMER_REPEAT:
						timer.active = true;
						timer.deadline += timer.interval;
						break;
					case VNL_TIMER_MANUAL:
					case VNL_TIMER_ONCE:
						timer.active = false;
						break;
				}
				timer.func();
				diff = timer.deadline - now;
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
	} else if(msg->msg_id == Pipe::close_t::MID) {
		Basic* pipe = ((Pipe::close_t*)msg)->data;
		pipes.remove(pipe);
		msg->ack();
		return true;
	}
	return false;
}

bool Object::handle(Packet* pkt) {
	int64_t& last_seq = sources[pkt->src_mac];
	if(last_seq == 0) {
		log(DEBUG).out << "New source: mac=" << hex(pkt->src_mac) << " num_hops=" << pkt->num_hops << endl;
	}
	if(pkt->seq_num <= last_seq) {
		if(pkt->pkt_id == Frame::PID) {
			Frame* result = buffer.create<Frame>();
			send_async(result, pkt->src_addr);
		}
		pkt->ack();
		return true;
	}
	last_seq = pkt->seq_num;
	
	if(pkt->pkt_id == vnl::Sample::PID) {
		Sample* sample = (Sample*)pkt->payload;
		if(sample->data) {
			if(handle_switch(sample->data, pkt)) {
				pkt->ack();
				return true;
			}
		}
	} else if(pkt->pkt_id == Frame::PID) {
		Frame* request = (Frame*)pkt->payload;
		Frame* result = buffer.create<Frame>();
		result->seq_num = request->seq_num;
		buf_in.wrap(request->data, request->size);
		input.reset();
		int size = 0;
		int id = input.getEntry(size);
		if(id == VNL_IO_INTERFACE) {
			uint32_t hash = 0;
			input.getHash(hash);
			while(!input.error()) {
				int size = 0;
				int id = input.getEntry(size);
				if(id == VNL_IO_CALL) {
					input.getHash(hash);
					bool res = vni_call(input, hash, size);
					if(!res) {
						input.skip(id, size, hash);
						log(WARN).out << "VNL_IO_CALL failed: hash=" << hex(hash) << " size=" << size << endl;
					}
				} else if(id == VNL_IO_CONST_CALL) {
					input.getHash(hash);
					result->data = Page::alloc();
					buf_out.wrap(result->data);
					if(!vni_const_call(input, hash, size, output)) {
						input.skip(id, size, hash);
						output.putNull();
						log(WARN).out << "VNL_IO_CONST_CALL failed: hash=" << hex(hash) << " size=" << size << endl;
					}
					output.flush();
				} else if(id == VNL_IO_INTERFACE && size == VNL_IO_END) {
					break;
				} else {
					input.skip(id, size);
				}
			}
			result->size = buf_out.position();
			buf_out.clear();
		}
		if(input.error()) {
			log(WARN).out << "Invalid Frame received: size=" << request->size << endl;
		}
		send_async(result, request->src_addr);
		pkt->ack();
		return true;
	}
	return false;
}

void Object::handle(const vnl::Shutdown& event) {
	dorun = false;
}

bool Object::sleep(int64_t secs) {
	return usleep(secs*1000*1000);
}

bool Object::usleep(int64_t micros) {
	int64_t now = currentTimeMicros();
	int64_t deadline = now + micros;
	while(dorun && now < deadline) {
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

void Object::exec(Engine* engine_, Message* init) {
	engine = engine_;
	stream.connect(engine_);
	subscribe(my_address);
	Announce* announce = Announce::create();
	announce->instance.type = type_name();
	announce->instance.domain = my_domain;
	announce->instance.topic = my_topic;
	publish(announce, local_domain_name, "vnl.announce");
	main(engine_, init);
	for(Basic* pipe : pipes) {
		Pipe::close_t msg(this);
		send(&msg, pipe);
	}
	publish(Exit::create(), local_domain_name, "vnl.exit");
	stream.close();
}

vnl::info::Class Object::get_class() const {
	vnl::info::Class res;
	res.name = type_name();
	// TODO
	return res;
}

Binary Object::vni_serialize() const {
	Binary blob;
	blob.data = Page::alloc();
	vnl::io::ByteBuffer buf;
	buf.wrap(blob.data);
	vnl::io::TypeOutput out(&buf);
	serialize(out);
	out.flush();
	blob.size = buf.position();
	return blob;
}

void Object::vni_deserialize(const vnl::Binary& blob) {
	vnl::io::ByteBuffer buf;
	vnl::io::TypeInput in(&buf);
	buf.wrap(blob.data, blob.size);
	vnl::read(in, *this);
}




}
