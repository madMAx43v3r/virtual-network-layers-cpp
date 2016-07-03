/*
 * Object.cpp
 *
 *  Created on: Mar 7, 2016
 *      Author: mad
 */

#include "vnl/Module.h"
#include "vnl/Registry.h"
#include "vnl/Util.h"


namespace vnl {

int Module::global_log_level = Module::INFO;

Module::Module()
	:	Module(Random64::global_rand())
{
}

Module::Module(uint64_t mac)
	:	log_writer(this)
{
	this->mac = mac;
	my_name << vnl::hex(mac);
}

Module::Module(const char* name)
	:	Module::Module(hash64(name))
{
	this->my_name = name;
}

Module::Module(const String& name)
	:	Module::Module(hash64(name))
{
	this->my_name = name;
}

void Module::receive(Message* msg) {
	stream.receive(msg);
}

void Module::die() {
	dying = true;
}

StringWriter Module::log(int level) {
	StringOutput* out = 0;
	if(level <= log_level) {
		out = &log_writer;
	}
	StringWriter writer(out);
	writer.out << "[" << my_name << "] ";
	switch(level) {
		case ERROR: writer.out << "ERROR: "; break;
		case WARN: writer.out << "WARNING: "; break;
		case INFO: writer.out << "INFO: "; break;
		case DEBUG: writer.out << "DEBUG: "; break;
	}
	return writer;
}

StringWriter Module::log(int level, double time) {
	StringWriter writer = log(level);
	writer.out << "(" << vnl::def(time, 3) << ") ";
	return writer;
}

StringWriter Module::log(int level, int64_t time) {
	StringWriter writer = log(level);
	writer.out << "(" << vnl::dec(time) << ") ";
	return writer;
}

Timer* Module::set_timeout(int64_t micros, const std::function<void(Timer*)>& func, Timer::type_t type) {
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

bool Module::poll(int64_t micros) {
	int64_t to = micros;
	int64_t now = currentTimeMicros();
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
				diff = timer->deadline - now;
			}
			if(diff < to || to == -1) {
				to = diff;
			}
		}
		timer = timer->next;
	}
	while(true) {
		Message* msg = stream.poll(to);
		if(!msg) {
			break;
		}
		if(msg->msg_id == Registry::exit_t::MID) {
			msg->ack();
			return false;
		} else {
			process(msg);
		}
		to = 0;
	}
	return true;
}

bool Module::sleep(int64_t secs) {
	return usleep(secs*1000*1000);
}

bool Module::usleep(int64_t micros) {
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

void Module::process(Message* msg) {
	switch(msg->msg_id) {
	case get_name_t::MID:
		((get_name_t*)msg)->data = my_name;
		msg->ack();
		break;
	case set_log_level_t::MID:
		log_level = ((set_log_level_t*)msg)->data;
		msg->ack();
		break;
	default:
		if(!handle(msg)) {
			msg->ack();
		}
	}
}

void Module::run() {
	while(!dying && poll(-1));
}

void Module::exec(Engine* engine_) {
	engine = engine_;
	stream.connect(engine_);
	Registry::bind_t bind(this);
	stream.send(&bind, Registry::instance);
	if(!bind.res) {
		log(ERROR).out << "Duplicate name: '" << my_name << "' is already in use!" << vnl::endl;
		return;
	}
	main(engine_);
	dying = true;
	stream.flush();
	while(true) {
		Message* msg = stream.poll(0);
		if(msg) {
			msg->ack();
		} else {
			break;
		}
	}
	Timer* timer = timer_begin;
	while(timer) {
		Timer* next = timer->next;
		timer->~Timer();
		timer = next;
	}
}




}
