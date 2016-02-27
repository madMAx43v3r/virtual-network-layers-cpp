/*
 * Object.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include "phy/Object.h"
#include <assert.h>

namespace vnl { namespace phy {

Object::Object() : engine(Engine::local), queue(this, 0), task(0) {
	if(!engine) {
		printf("ERROR: creating vnl::phy::Object without thread local engine.\n");
		assert(Engine::local != 0);
	}
	mac = rand();
}

Object::Object(Engine* engine) : engine(engine) {
	mac = rand();
}

void Object::run() {
	while(true) {
		Message* msg = queue.poll(10);
		if(!msg) {
			break;
		}
		if(!handle(msg)) {
			msg->ack();
		}
	}
	task = 0;
}

void Object::receive(Message* msg, Object* src) {
	if(src == engine) {
		if(msg->isack) {
			if(msg->callback) {
				msg->callback(msg);
			}
			engine->ack(msg);
		} else {
			Stream* stream = get_stream(msg->sid);
			if(stream) {
				stream->receive(msg, this);
				engine->handle(msg, stream);
				if(msg->sid == 0 && !task) {
					task = launch(this);
				}
			}
		}
	} else {
		engine->receive(msg, src);
	}
}

Stream* Object::get_stream(uint64_t sid) {
	auto iter = streams.find(sid);
	if(iter != streams.end()) { return iter->second; }
	return 0;
}


}}
