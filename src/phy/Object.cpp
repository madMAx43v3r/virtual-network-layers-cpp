/*
 * Object.cpp
 *
 *  Created on: Mar 7, 2016
 *      Author: mad
 */

#include "vnl/phy/Object.h"
#include "vnl/phy/Registry.h"
#include "vnl/Util.h"


namespace vnl { namespace phy {

int Object::global_log_level = Object::INFO;

Object::Object() : Object((uint64_t)0) {}

Object::Object(uint64_t mac)
	:	buffer(memory), my_name(memory)
{
	this->mac = mac;
}

Object::Object(const char* name)
	:	Object::Object(vnl::hash64(name))
{
	this->my_name = name;
}

Object::Object(const vnl::String& name)
	:	Object::Object(vnl::hash64(name))
{
	this->my_name = name;
}

void Object::die() {
	Registry::delete_t msg(this);
	stream->send(&msg, Registry::instance);
}

vnl::StringWriter Object::log(int level) {
	vnl::StringOutput* out = 0;
	if(level <= log_level) {
		out = &vnl::cout;
	}
	vnl::StringWriter writer(out);
	writer.out << "[" << my_name << "] ";
	switch(level) {
		case ERROR: writer.out << "ERROR: "; break;
		case WARN: writer.out << "WARNING: "; break;
		case INFO: writer.out << "INFO: "; break;
	}
	return writer;
}

Timer* Object::timeout(int64_t micros, const std::function<void(Timer*)>& func, Timer::type_t type) {
	Timer* timer = timer_begin;
	while(timer) {
		if(timer->free) {
			timer->free = false;
			break;
		}
		timer = timer->next;
	}
	if(!timer) {
		timer = memory.create<Timer>();
		timer->next = timer_begin;
		timer_begin = timer;
	}
	timer->interval = micros;
	timer->func = func;
	timer->type = type;
	timer->reset();
	return timer;
}

void Object::run() {
	while(true) {
		int64_t to = -1;
		int64_t now = vnl::System::currentTimeMicros();
		Timer* timer = timer_begin;
		while(timer) {
			if(timer->active) {
				int64_t diff = timer->deadline - now;
				if(diff <= 0) {
					timer->active = false;
					timer->func(timer);
					switch(timer->type) {
						case Timer::REPEAT: timer->active = true;
									 timer->deadline += timer->interval;	break;
						case Timer::MANUAL: 								break;
						case Timer::ONCE: timer->destroy(); 				break;
					}
				} else if(diff < to || to == -1) {
					to = diff;
				}
			}
			timer = timer->next;
		}
		Message* msg = stream->poll(to);
		if(!msg) {
			continue;
		}
		if(msg->msg_id == Registry::exit_t::MID) {
			msg->ack();
			break;
		} else {
			if(!handle(msg)) {
				msg->ack();
			}
		}
	}
}

void Object::exec(Engine* engine_) {
	engine = engine_;
	Stream tmp(engine, memory);
	stream = &tmp;
	if(mac == 0) {
		mac = engine->rand();
		my_name << vnl::hex(mac);
	}
	Registry::bind_t bind(this);
	stream->send(&bind, Registry::instance);
	if(!bind.res) {
		log(ERROR).out << "Duplicate name: '" << my_name << "' is already in use!" << vnl::endl;
		return;
	}
	main(engine_);
	stream->flush();
	while(true) {
		Message* msg = stream->poll(0);
		if(msg) {
			msg->ack();
		} else {
			break;
		}
	}
}




}}
