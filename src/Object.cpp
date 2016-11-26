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

void Object::exit() {
	vnl_dorun = false;
}

StringWriter Object::log(int level) {
	StringOutput* out = 0;
	if(level <= vnl_log_level) {
		vnl_log_writer.level = level;
		out = &vnl_log_writer;
	}
	return StringWriter(out);
}

Timer* Object::set_timeout(int64_t micros, const std::function<void()>& func, int type) {
	Timer& timer = *vnl_timers.push_back();
	timer.interval = micros;
	timer.func = func;
	timer.type = type;
	if(type != VNL_TIMER_MANUAL) {
		timer.reset();
	}
	return &timer;
}

Object* Object::fork(Object* object) {
	vnl_engine->fork(object);
	return object;
}

Address Object::subscribe(const String& domain, const String& topic) {
	return subscribe(Address(domain, topic));
}

Address Object::subscribe(Address address) {
	return vnl_stream.subscribe(address);
}

void Object::unsubscribe(Hash64 domain, Hash64 topic) {
	unsubscribe(Address(domain, topic));
}

void Object::unsubscribe(Address address) {
	vnl_stream.unsubscribe(address);
}

void Object::publish(Value* data, const String& domain, const String& topic) {
	Header* header = Header::create();
	header->send_time = vnl::currentTimeMicros();
	header->src_mac = vnl_stream.get_mac();
	header->src_topic.domain = my_domain;
	header->src_topic.name = my_topic;
	header->dst_topic.domain = domain;
	header->dst_topic.name = topic;
	Sample* pkt = vnl_buffer.create<Sample>();
	pkt->header = header;
	pkt->data = data;
	send_async(pkt, Address(domain, topic));
}

void Object::publish(Value* data, Address topic) {
	Header* header = Header::create();
	header->send_time = vnl::currentTimeMicros();
	header->src_mac = vnl_stream.get_mac();
	Sample* pkt = vnl_buffer.create<Sample>();
	pkt->header = header;
	pkt->data = data;
	send_async(pkt, topic);
}

void Object::send(Packet* packet, Address dst) {
	if(packet->src_addr.is_null()) {
		packet->src_addr = my_address;
	}
	vnl_stream.send(packet, dst);
}

void Object::send_async(Packet* packet, Address dst) {
	if(packet->src_addr.is_null()) {
		packet->src_addr = my_address;
	}
	vnl_stream.send_async(packet, dst);
}

void Object::send(Packet* packet, Basic* dst) {
	if(packet->src_addr.is_null()) {
		packet->src_addr = my_address;
	}
	vnl_stream.send(packet, dst);
}

void Object::send_async(Packet* packet, Basic* dst) {
	if(packet->src_addr.is_null()) {
		packet->src_addr = my_address;
	}
	vnl_stream.send_async(packet, dst);
}

void Object::send(Message* msg, Basic* dst) {
	vnl_stream.send(msg, dst);
}

void Object::send_async(Message* msg, Basic* dst) {
	vnl_stream.send_async(msg, dst);
}

void Object::attach(Pipe* pipe) {
	vnl_pipes.push_back(pipe);
	pipe->ack(&vnl_stream);
}

void Object::close(Pipe* pipe) {
	vnl_pipes.remove(pipe);
	pipe->fin();
}

bool Object::poll(int64_t micros) {
	int64_t to = micros;
	int64_t now = currentTimeMicros();
	for(Timer& timer : vnl_timers) {
		if(timer.active) {
			int64_t diff = timer.deadline - now;
			if(diff <= 0) {
				switch(timer.type) {
					case VNL_TIMER_REPEAT:
						timer.active = true;
						timer.deadline += timer.interval;
						if(timer.deadline < now) {
							timer.deadline = now;
						}
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
		Message* msg = vnl_stream.poll(to);
		if(!msg) {
			break;
		}
		if(!handle(msg)) {
			msg->ack();
		}
		to = 0;
	}
	return vnl_dorun;
}

bool Object::handle(Message* msg) {
	if(msg->msg_id == Packet::MID) {
		Packet* pkt = (Packet*)msg;
		return handle(pkt);
	} else if(msg->msg_id == Pipe::close_t::MID) {
		Basic* pipe = ((Pipe::close_t*)msg)->data;
		vnl_pipes.remove(pipe);
		msg->ack();
		return true;
	}
	return false;
}

bool Object::handle(Packet* pkt) {
	int64_t& last_seq = vnl_sources[pkt->src_mac];
	if(last_seq == 0) {
		log(DEBUG).out << "New source: mac=" << hex(pkt->src_mac) << " num_hops=" << pkt->num_hops << endl;
	}
	if(pkt->seq_num <= last_seq) {
		if(pkt->pkt_id == Frame::PID) {
			Frame* result = vnl_buffer.create<Frame>();
			result->req_num = ((Frame*)pkt->payload)->req_num;
			send_async(result, pkt->src_addr);
		}
		pkt->ack();
		return true;
	}
	last_seq = pkt->seq_num;
	
	if(pkt->pkt_id == Sample::PID) {
		Sample* sample = (Sample*)pkt->payload;
		if(handle(sample)) {
			pkt->ack();
			return true;
		}
	} else if(pkt->pkt_id == Frame::PID) {
		Frame* frame = (Frame*)pkt->payload;
		if(handle(frame)) {
			pkt->ack();
			return true;
		}
	}
	return false;
}

bool Object::handle(Sample* sample) {
	if(sample->data) {
		return handle_switch(sample->data, sample);
	}
	return false;
}

bool Object::handle(Frame* frame) {
	Frame* result = vnl_buffer.create<Frame>();
	result->type = Frame::RESULT;
	result->req_num = frame->req_num;
	vnl_buf_in.wrap(frame->data, frame->size);
	vnl_input.reset();
	uint32_t hash;
	int size = 0;
	int id = vnl_input.getEntry(size);
	if(id == VNL_IO_CALL) {
		vnl_input.getHash(hash);
		bool res = vni_call(vnl_input, hash, size);
		if(!res) {
			vnl_input.skip(id, size, hash);
			log(WARN).out << "VNL_IO_CALL failed: hash=" << hex(hash) << " size=" << size << endl;
		}
	} else if(id == VNL_IO_CONST_CALL) {
		vnl_input.getHash(hash);
		result->data = Page::alloc();
		vnl_buf_out.wrap(result->data);
		if(!vni_const_call(vnl_input, hash, size, vnl_output)) {
			vnl_input.skip(id, size, hash);
			vnl_output.putNull();
			log(WARN).out << "VNL_IO_CONST_CALL failed: hash=" << hex(hash) << " size=" << size << endl;
		}
		vnl_output.flush();
	} else {
		vnl_input.skip(id, size);
	}
	result->size = vnl_buf_out.position();
	vnl_buf_out.clear();
	if(vnl_input.error()) {
		log(WARN).out << "Invalid Frame received: size=" << frame->size << endl;
	}
	send_async(result, frame->src_addr);
	return true;
}

void Object::handle(const vnl::Shutdown& event) {
	vnl_dorun = false;
}

bool Object::sleep(int64_t secs) {
	return usleep(secs*1000*1000);
}

bool Object::usleep(int64_t micros) {
	int64_t now = currentTimeMicros();
	int64_t deadline = now + micros;
	while(vnl_dorun && now < deadline) {
		int64_t to = deadline - now;
		if(!poll(to)) {
			return false;
		}
		now = currentTimeMicros();
	}
	return true;
}

void Object::run() {
	while(vnl_dorun && poll(-1));
}

void Object::exec(Engine* engine_, Message* init, Pipe* pipe) {
	vnl_dorun = true;
	vnl_engine = engine_;
	vnl_stream.connect(engine_);
	if(pipe) {
		attach(pipe);
	}
	subscribe(my_address);
	Announce* announce = Announce::create();
	announce->instance.type = type_name();
	announce->instance.domain = my_domain;
	announce->instance.topic = my_topic;
	publish(announce, local_domain_name, "vnl.announce");
	main(engine_, init);
	for(Basic* pipe : vnl_pipes) {
		Pipe::close_t msg(&vnl_stream);
		send(&msg, pipe);
	}
	publish(Exit::create(), local_domain_name, "vnl.exit");
	vnl_stream.close();
}


} // vnl
