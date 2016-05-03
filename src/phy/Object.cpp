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


Object::Object() : mac(0) {}

Object::Object(uint64_t mac) : mac(mac) {}

Object::Object(const std::string& name)
	:	Object::Object(Util::hash64(name)), name(name)
{
}

Object::Object(Object* parent, const std::string& name)
	:	Object::Object(parent->name + name)
{
}

void Object::die() {
	send(Registry::delete_t(this), Registry::instance);
}

void Object::mainloop(Engine* engine) {
	Stream s(engine);
	stream = &s;
	if(mac == 0) {
		mac = stream->rand();
	}
	assert(request<bool>(Registry::bind_t(this), Registry::instance));
	while(true) {
		Message* msg = stream->poll();
		if(!msg) {
			break;
		}
		if(msg->mid == exit_t::id) {
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
