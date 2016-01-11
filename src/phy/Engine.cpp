/*
 * Engine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include "phy/Engine.h"

namespace vnl { namespace phy {

static thread_local Engine* Engine::local = 0;

Engine::Engine() : Object(this), thread(0), core_id(-1) {
	static std::atomic<int> counter;
	generator.seed(Util::hash64(counter++, System::nanoTime()));
}

void Engine::start(int core) {
	dorun = true;
	if(!thread) {
		core_id = core;
		thread = new std::thread(&Engine::entry, this);
	}
}

void Engine::stop() {
	dorun = false;
	if(thread) {
		Object::receive(new shutdown_t(0, true));
		thread->join();
		delete thread;
		thread = 0;
	}
}

void Engine::run() {
	local = this;
	if(core_id >= 0) {
		Util::stick_to_core(core_id);
	}
	if(!startup()) {
		return;
	}
	while(dorun) {
		Message* msg = poll();
		if(msg) {
			if(debug > 0) {
				std::cout << std::dec << System::currentTimeMillis() << " Engine@" << this << " "
						<< (msg->isack ? "ACK" : "RCV") << " " << msg->toString() << std::endl;
			}
			receive(msg, this);
		}
	}
	shutdown();
}

void Engine::receive(Message* msg, Object* src) {
	if(src == this) {
		Object* dst;
		if(msg->isack) {
			dst = msg->src;
		} else {
			dst = msg->dst;
		}
		if(dst == this) {
			Object::receive(msg, this);
		} else {
			dst->receive(msg, this);
		}
	} else {
		lock();
			if(msg->isack) {
				acks.push(msg);
			} else {
				queue.push(msg);
			}
		notify();
		unlock();
	}
}

Message* Engine::poll() {
	lock();
	Message* msg = 0;
	while(dorun) {
		if(acks.pop(msg) || queue.pop(msg)) {
			break;
		} else {
			wait(engine->timeout());
		}
	}
	unlock();
	return msg;
}

void Engine::forward(Message* msg) {
	msg->dst->receive(msg, msg->src);
}

void Engine::dispatch(Message* msg) {
	msg->dst->handle(msg);
}

void Engine::dispatch(Message* msg, Stream* stream) {
	stream->receive(msg, msg->dst);
}


}}
