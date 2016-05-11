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
	:	buffer(memory)
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

void Object::run() {
	while(true) {
		Message* msg = stream->poll();
		if(!msg) {
			break;
		}
		if(msg->mid == exit_t::MID) {
			exit_msg = msg;
			break;
		}
		if(!handle(msg)) {
			msg->ack();
		}
	}
}

void Object::main(Engine* engine_) {
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
	main();
	stream->flush();
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
