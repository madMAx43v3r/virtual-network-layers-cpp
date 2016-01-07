/*
 * Link.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include "phy/Link.h"

namespace vnl { namespace phy {

Link::~Link() {
	delete engine;
}

void Link::start(int core) {
	dorun = true;
	if(!thread) {
		core_id = core;
		thread = new std::thread(&Link::entry, this);
	}
}

void Link::stop() {
	dorun = false;
	if(thread) {
		Object::receive(new shutdown_t(0, true));
		thread->join();
		delete thread;
		thread = 0;
	}
}

void Link::run() {
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
				std::cout << std::dec << System::currentTimeMillis() << " Link@" << this << " "
						<< (msg->isack ? "ACK" : "RCV") << " " << msg->toString() << std::endl;
			}
			receive(msg, this);
		}
	}
	shutdown();
}

void Link::receive(Message* msg, Object* src) {
	if(src == this) {
		if(msg->isack) {
			if(msg->src == this) {
				Object::receive(msg, this);
			} else {
				msg->src->receive(msg, this);
			}
		} else {
			if(msg->dst == this) {
				Object::receive(msg, this);
			} else {
				msg->dst->receive(msg, this);
			}
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

Message* Link::poll() {
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


}}
