/*
 * Stream.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include "phy/Stream.h"

namespace vnl { namespace phy {

Stream::Stream(Object* object) : Stream(object, object->rand()) {}

Stream::Stream(Object*object , uint64_t sid) : obj(object), sid(sid) {
	obj->open(this);
}

Stream::~Stream() {
	obj->close(this);
	for(Message* msg : queue) {
		msg->ack();
	}
}

void Stream::send(Message* msg, bool async) {
	msg->sid = sid;
	obj->send(msg, async);
}

void Stream::receive(Message* msg) {
	msg->sid = sid;
	obj->receive(msg);
}

void Stream::receive(Message* msg, Object* src) {
	if(src == obj) {
		queue.push(msg);
	} else {
		obj->receive(msg, src);
	}
}

Message* Stream::poll() {
	while(true) {
		Message* msg = poll(2147483647);
		if(msg) {
			return msg;
		}
	}
}

Message* Stream::poll(int millis) {
	Message* msg = 0;
	if(!queue.pop(msg) && millis >= 0) {
		if(obj->engine->poll(this, millis)) {
			queue.pop(msg);
		}
	}
	return msg;
}


}}
