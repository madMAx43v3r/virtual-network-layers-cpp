/*
 * Object.cpp
 *
 *  Created on: Mar 7, 2016
 *      Author: mad
 */

#include "phy/Object.h"
#include "phy/Registry.h"
#include "Util.h"


namespace vnl { namespace phy {


Object::Object() : Object(0) {}

Object::Object(uint64_t mac)
	:	buffer(memory), timers(memory)
{
	this->mac = mac;
}

Object::Object(const vnl::String& name)
	:	Object::Object(Util::hash64(name))
{
	this->name = name;
}

Object::Object(Object* parent, const vnl::String& name)
	:	Object::Object(vnl::String() << parent->name << name)
{
}

void Object::die() {
	Registry::delete_t msg(this);
	stream->send(&msg, Registry::instance);
}

void Object::exit(Message* msg) {
	exit_msg = msg;
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
		timer = timers.create();
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
		if(msg->msg_id == exit_t::MID) {
			exit(msg);
			break;
		}
		if(!handle(msg)) {
			msg->ack();
		}
	}
}

void Object::exec(Engine* engine_) {
	engine = engine_;
	Stream tmp(engine, memory);
	stream = &tmp;
	if(mac == 0) {
		mac = engine->rand();
	}
	Registry::bind_t bind(this);
	stream->send(&bind, Registry::instance);
	if(!bind.res) {
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
	if(exit_msg) {
		exit_msg->ack();
	}
}


Reference::Reference(Engine* engine, Object* obj)
	:	mac(obj->getMAC()), engine(engine), obj(obj)
{
	Registry::open_t msg(obj);
	engine->send(engine, &msg, Registry::instance);
}

Reference::Reference(Engine* engine, uint64_t mac)
	:	mac(mac), engine(engine)
{
}

Reference::Reference(Engine* engine, const vnl::String& name) 
	:	Reference(engine, Util::hash64(name))
{
}

Reference::Reference(Engine* engine, Object* parent, const vnl::String& name)
	:	Reference(engine, vnl::String() << parent->getName() << name)
{
}

Object* Reference::get() {
	if(!obj) {
		Registry::connect_t req(mac);
		engine->send(engine, &req, Registry::instance);
		obj = req.res;
	}
	return obj;
}

void Reference::close() {
	if(obj) {
		Registry::close_t msg(obj);
		engine->send(engine, &msg, Registry::instance);
		obj = 0;
	}
}


}}
