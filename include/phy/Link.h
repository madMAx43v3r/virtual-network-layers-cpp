/*
 * Link.h
 *
 *  Created on: Dec 23, 2015
 *      Author: mad
 */

#ifndef INCLUDE_LINK_H_
#define INCLUDE_LINK_H_

#include <thread>
#include <mutex>
#include <iostream>

#include "util/simple_queue.h"
#include "Object.h"
#include "System.h"

namespace vnl { namespace phy {

class Link : public vnl::phy::Object, public vnl::Runnable {
public:
	Link(Engine* engine) : Object(engine), thread(0) {}
	
	int debug = 0;
	
	virtual ~Link() {
		delete engine;
	}
	
	void start(int core = -1) {
		dorun = true;
		if(!thread) {
			core_id = core;
			thread = new std::thread(&Link::entry, this);
		}
	}
	
	void stop() {
		dorun = false;
		if(thread) {
			Object::receive(new shutdown_t(0, true));
			thread->join();
			delete thread;
			thread = 0;
		}
	}
	
	void run() override {
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
	
protected:
	bool dorun = true;
	std::mutex mutex;
	
	typedef Generic<int, 0x2fe94e9f> shutdown_t;
	
	void lock() {
		mutex.lock();
	}
	
	void unlock() {
		mutex.unlock();
	}
	
	virtual void notify() = 0;
	virtual void wait(int millis) = 0;
	
	virtual bool startup() { return true; }
	virtual void shutdown() {}
	
	void receive(Message* msg, Object* src) override {
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
	
	Message* poll() {
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
	
private:
	void entry() {
		run();
	}
	
private:
	vnl::util::simple_queue<Message*> queue;
	vnl::util::simple_queue<Message*> acks;
	
	int core_id = -1;
	std::thread* thread;
	
};

}}

#endif /* INCLUDE_LINK_H_ */
