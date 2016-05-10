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
	stream->send(Registry::delete_t(this), Registry::instance);
}

void Object::run(Engine* engine_) {
	engine = engine_;
	Stream tmp(engine, memory);
	stream = &tmp;
	if(mac == 0) {
		mac = engine->rand();
	}
	if(!stream->request<bool>(Registry::bind_t(this), Registry::instance)) {
		return;
	}
	if(!startup()) {
		stream->flush();
		return;
	}
	while(true) {
		Message* msg = stream->poll();
		if(!msg) {
			break;
		}
		if(msg->mid == exit_t::id) {
			std::cout << "Object " << this << " got exit_t message" << std::endl;
			shutdown();
			stream->flush();
			msg->ack();
			break;
		}
		if(!handle(msg)) {
			msg->ack();
		}
	}
}


Reference::Reference(Engine* engine, Object* obj)
	:	mac(obj->getMAC()), engine(engine), obj(obj)
{
	engine->send_async(Registry::open_t(obj), Registry::instance);
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
		obj = engine->request<Object*>(Registry::connect_t(mac), Registry::instance);
	}
	return obj;
}

void Reference::close() {
	if(obj) {
		engine->send_async(Registry::close_t(obj), Registry::instance);
		obj = 0;
	}
}


}}
