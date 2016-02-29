/*
 * Object.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include "phy/Object.h"
#include "phy/Engine.h"
#include <assert.h>

namespace vnl { namespace phy {

Object::Object() : engine(Engine::local), queue(this, 0), running(0) {
	if(!engine) {
		printf("ERROR: creating vnl::phy::Object without thread local engine.\n");
		assert(Engine::local != 0);
	}
	mac = rand();
	main = std::bind(&Object::mainloop, this);
}

void Object::receive(Message* msg) {
	msg->dst = this;
	engine->receive(msg);
}

uint64_t Object::rand() {
	return engine->rand();
}

void Object::send(Message* msg, Object* dst, bool async) {
	msg->src = this;
	msg->dst = dst;
	msg->async = async;
	if(dst == this) {
		msg->src = 0;
		handle(msg);
	} else {
		msg->dst->receive(msg);
		engine->sent(msg);
	}
}

void Object::open(Stream* stream) {
	streams[stream->sid] = stream;
}

void Object::close(Stream* stream) {
	streams.erase(stream->sid);
}

void Object::flush() {
	engine->flush();
}

void Object::yield() {
	int64_t now = System::currentTimeMillis();
	if(now - last_yield >= 10) {
		sleep(0);
		last_yield = now;
	}
}

void Object::sleep(int millis) {
	Stream stream(this);
	stream.poll(millis);
}

taskid_t Object::launch(Runnable* task) {
	return engine->launch(std::bind(&Runnable::run, task));
}

taskid_t Object::launch(const std::function<void()>& func) {
	return engine->launch(func);
}

void Object::cancel(taskid_t task) {
	engine->cancel(task);
}

void Object::mainloop() {
	running = true;
	while(true) {
		Message* msg = queue.poll(10);
		if(!msg) {
			break;
		}
		if(!handle(msg)) {
			msg->ack();
		}
	}
	running = false;
}


}}
