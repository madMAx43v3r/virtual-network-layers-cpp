/*
 * Stream.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include "phy/Stream.h"
#include "phy/Object.h"
#include "phy/Engine.h"

namespace vnl { namespace phy {

Stream::Stream(Object* obj) : Stream(obj, obj->rand()) {}

Stream::Stream(Object* obj , uint64_t sid) : obj(obj), sid(sid) {
	obj->open(this);
}

Stream::~Stream() {
	obj->close(this);
	for(Message* msg : queue) {
		msg->ack();
	}
}

void Stream::send(Message* msg, Object* dst, bool async) {
	msg->sid = sid;
	obj->send(msg, dst, async);
}

void Stream::receive(Message* msg) {
	msg->sid = sid;
	obj->receive(msg);
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
