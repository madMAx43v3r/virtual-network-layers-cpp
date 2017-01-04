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
#include <vnl/Heartbeat.hxx>
#include <vnl/Exit.hxx>
#include <vnl/NoSuchMethodException.hxx>


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

void Object::add_client(Client& client, Router* target) {
	assert(vnl_dorun);
	client.connect(vnl_engine, target);
	vnl_output_channels[client.get_mac()] = "Client";
}

void Object::add_input(Stream& stream, Router* target) {
	assert(vnl_dorun);
	stream.connect(vnl_engine, target);
	stream.listen(this);
}

void Object::add_input(InputPin& pin) {
	assert(vnl_dorun);
	pin.enable(vnl_engine, this);
	vnl_input_pins[pin.get_mac()] = &pin;
}

void Object::add_output(OutputPin& pin) {
	assert(vnl_dorun);
	pin.enable(vnl_engine);
	vnl_output_pins[pin.get_mac()] = &pin;
	vnl_output_channels[pin.get_mac()] = "OutputPin";
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

void Object::publish(Value* data, const String& domain, const String& topic, bool no_drop) {
	Header* header = Header::create();
	header->send_time = vnl::currentTimeMicros();
	header->src_mac = vnl_stream.get_mac();
	header->src_topic.domain = my_domain;
	header->src_topic.name = my_topic;
	header->dst_topic.domain = domain;
	header->dst_topic.name = topic;
	Sample* pkt = vnl_sample_buffer.create();
	pkt->header = header;
	pkt->data = data;
	send_async(pkt, Address(domain, topic), no_drop);
}

void Object::publish(Value* data, Address topic, bool no_drop) {
	Header* header = Header::create();
	header->send_time = vnl::currentTimeMicros();
	header->src_mac = vnl_stream.get_mac();
	Sample* pkt = vnl_sample_buffer.create();
	pkt->header = header;
	pkt->data = data;
	send_async(pkt, topic, no_drop);
}

void Object::send(Packet* packet, Address dst, bool no_drop) {
	if(packet->src_addr.is_null()) {
		packet->src_addr = my_address;
	}
	vnl_stream.send(packet, dst, no_drop);
}

void Object::send_async(Packet* packet, Address dst, bool no_drop) {
	if(packet->src_addr.is_null()) {
		packet->src_addr = my_address;
	}
	vnl_stream.send_async(packet, dst, no_drop);
}

void Object::send(Packet* packet, Basic* dst, bool no_drop) {
	if(packet->src_addr.is_null()) {
		packet->src_addr = my_address;
	}
	vnl_stream.send(packet, dst, no_drop);
}

void Object::send_async(Packet* packet, Basic* dst, bool no_drop) {
	if(packet->src_addr.is_null()) {
		packet->src_addr = my_address;
	}
	vnl_stream.send_async(packet, dst, no_drop);
}

void Object::send(Message* msg, Basic* dst, bool no_drop) {
	vnl_stream.send(msg, dst, no_drop);
}

void Object::send_async(Message* msg, Basic* dst, bool no_drop) {
	vnl_stream.send_async(msg, dst, no_drop);
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
	vnl_input_nodes[msg->src_mac]++;
	vnl_input_channels[vnl_channel->get_mac()]++;
	if(msg->msg_id == Packet::MID) {
		Packet* pkt = (Packet*)msg;
		if(pkt->proxy) {
			vnl_input_nodes[pkt->proxy]++;
		}
		return handle(pkt);
	}
	if(msg->msg_id == OutputPin::pin_data_t::MID) {
		return handle((OutputPin::pin_data_t*)msg);
	} else if(msg->msg_id == Stream::notify_t::MID) {
		return handle((Stream::notify_t*)msg);
	}
	return false;
}

bool Object::handle(Stream::notify_t* notify) {
	Message* msg;
	while(notify->data->pop(msg)) {
		Stream* tmp_channel = vnl_channel;
		vnl_channel = notify->data;
		if(!handle(msg)) {
			msg->ack();
		}
		vnl_channel = tmp_channel;
	}
	return false;
}

bool Object::handle(OutputPin::pin_data_t* msg) {
	if(msg->data && handle_switch(msg->data, msg->dst)) {
		msg->ack();
		return true;
	}
	return false;
}

bool Object::handle(Packet* pkt) {
	int64_t& last_seq = vnl_sources[pkt->src_mac xor vnl_channel->get_mac()];
	if(pkt->seq_num <= last_seq) {
		if(pkt->pkt_id == Frame::PID) {
			Frame* result = vnl_frame_buffer.create();
			result->req_num = ((Frame*)pkt->payload)->req_num;
			send_async(result, pkt->src_addr);
		}
		pkt->ack();
		return true;
	}
	last_seq = pkt->seq_num;
	
	uint64_t tmp_proxy = vnl_proxy;
	vnl_proxy = pkt->proxy;
	bool res = false;
	if(pkt->pkt_id == Sample::PID) {
		res = handle((Sample*)pkt->payload);
	} else if(pkt->pkt_id == Frame::PID) {
		res = handle((Frame*)pkt->payload);
	}
	vnl_proxy = tmp_proxy;
	return res;
}

bool Object::handle(Sample* sample) {
	if(sample->data) {
		handle_switch(sample->data, sample);
	}
	return false;
}

bool Object::handle(Frame* frame) {
	Frame* result = exec_vni_call(frame);
	if(result) {
		send_async(result, frame->src_addr, true);
	}
	vnl::info::ClientInfo& info = vnl_clients[frame->src_mac];
	info.proxy = frame->proxy;
	info.num_requests++;
	info.num_errors += !result || result->type == Frame::EXCEPTION;
	return false;
}

Frame* Object::exec_vni_call(Frame* frame) {
	Frame* result = vnl_frame_buffer.create();
	result->type = Frame::RESULT;
	result->req_num = frame->req_num;
	result->data = Page::alloc();
	vnl_buf_out.wrap(result->data);
	vnl_buf_in.wrap(frame->data, frame->size);
	vnl_output.reset();
	vnl_input.reset();
	try {
		uint32_t hash;
		int size = 0;
		int id = vnl_input.getEntry(size);
		if(id == VNL_IO_CALL) {
			vnl_input.getHash(hash);
			bool res = vni_call(vnl_input, hash, size);
			if(!res) {
				throw NoSuchMethodException();
			}
		} else if(id == VNL_IO_CONST_CALL) {
			vnl_input.getHash(hash);
			if(!vni_const_call(vnl_input, hash, size, vnl_output)) {
				throw NoSuchMethodException();
			}
		} else {
			throw IOException();
		}
		if(vnl_input.error()) {
			throw IOException();
		}
	} catch (const Exception& ex) {
		result->type = Frame::EXCEPTION;
		vnl::write(vnl_output, ex);
	}
	vnl_output.flush();
	result->size = vnl_buf_out.position();
	vnl_buf_out.clear();
	return result;
}

String Object::get_private_domain() const {
	return my_private_domain;
}

Map<String, String> Object::get_config_map() const {
	Map<String, String> res;
	for(int i = 0; i < get_num_fields(); ++i) {
		get_field(i , res[get_field_name(i)]);
	}
	return res;
}

String Object::get_config(const Hash32& name) const {
	String res;
	get_field(get_field_index(name), res);
	return res;
}

void Object::set_config(const Hash32& name, const String& value) {
	set_field(get_field_index(name), value);
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
	vnl_spawn_time = vnl::currentTimeMicros();
	vnl_dorun = true;
	vnl_engine = engine_;
	vnl_stream.connect(engine_);
	vnl_stream.set_timeout(vnl_msg_timeout);
	vnl_channel = &vnl_stream;
	if(pipe) {
		pipe->open(this);
	}
	subscribe(my_address);
	subscribe(Address(my_address.domain(), get_mac()));
	
	Announce* announce = Announce::create();
	announce->instance.type = get_type_name();
	announce->instance.domain = my_domain;
	announce->instance.topic = my_topic;
	announce->instance.src_mac = get_mac();
	announce->instance.heartbeat_interval = vnl_heartbeat_interval;
	publish(announce, local_domain_name, "vnl.announce", true);
	
	set_timeout(vnl_heartbeat_interval, std::bind(&Object::heartbeat, this), VNL_TIMER_REPEAT);
	
	main(engine_, init);
	
	log(DEBUG).out << "Messages: num_sent=" << vnl_engine->num_sent << ", num_received="
			<< vnl_engine->num_received << ", num_timeout=" << vnl_engine->num_timeout << vnl::endl;
	
	for(InputPin* pin : vnl_input_pins.values()) {
		pin->close();
	}
	for(OutputPin* pin : vnl_output_pins.values()) {
		pin->close();
	}
	publish(Exit::create(), local_domain_name, "vnl.exit", true);
	
	if(pipe) {
		pipe->close();
	}
	vnl_stream.close();
}

void Object::heartbeat() {
	Heartbeat* msg = Heartbeat::create();
	msg->src_mac = get_mac();
	msg->type = get_type_name();
	msg->domain = my_domain;
	msg->topic = my_topic;
	msg->interval = vnl_heartbeat_interval;
	msg->info.time = vnl::currentTimeMicros();
	msg->info.spawn_time = vnl_spawn_time;
	msg->info.num_cycles = vnl_engine->num_cycles;
	msg->info.num_msg_sent = vnl_engine->num_sent;
	msg->info.num_msg_received = vnl_engine->num_received;
	msg->info.num_msg_dropped = vnl_engine->num_timeout;
	msg->info.sources = vnl_sources;
	msg->info.input_nodes = vnl_input_nodes;
	msg->info.input_channels = vnl_input_channels;
	msg->info.output_channels = vnl_output_channels;
	for(const auto& entry : vnl_input_pins) {
		msg->info.input_pins[entry.first] = entry.second->name;
	}
	for(const auto& entry : vnl_output_pins) {
		msg->info.output_pins[entry.first] = entry.second->name;
	}
	msg->info.clients = vnl_clients;
	publish(msg, local_domain_name, "vnl.heartbeat", true);
}


} // vnl
