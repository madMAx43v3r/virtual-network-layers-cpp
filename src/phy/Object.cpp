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

Object::Object()
	:	mac(engine->rand())
{
	bind();
}

Object::Object(uint64_t mac)
	:	mac(mac)
{
	bind();
}

Object::Object(const std::string& name)
	:	Object::Object(Util::hash64(name)), name(name)
{
}

Object::Object(Object* parent, const std::string& name)
	:	Object::Object(parent->name + name)
{
}

void Object::bind() {
	assert(Registry::instance);
	assert(Stream::request<bool>(Registry::bind_t(this), Registry::instance));
	task = engine->launch(std::bind(&Object::mainloop, this));
}

void Object::die() {
	Stream::send(Registry::kill_t(this), Registry::instance);
}

void Object::mainloop() {
	while(true) {
		Message* msg = poll();
		if(!msg) {
			break;
		}
		if(msg->mid == delete_t::id) {
			engine->listen_on(task, Registry::instance);
			die();
		} else if(msg->mid == exit_t::id) {
			shutdown();
			msg->ack();
			break;
		}
		if(!handle(msg)) {
			msg->ack();
		}
	}
}


}}
