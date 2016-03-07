/*
 * Stream.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_STREAM_H_
#define INCLUDE_PHY_STREAM_H_

#include <assert.h>

#include "phy/Engine.h"
#include "phy/Memory.h"
#include "util/queue.h"

namespace vnl { namespace phy {

class FiberEngine;
class ThreadEngine;


class Stream {
public:
	Stream() : engine(Engine::local) {
		assert(engine);
		mac = engine->rand();
	}
	
	Stream(uint64_t mac) : engine(Engine::local), mac(mac) {
		assert(engine);
	}
	
	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;
	
	typedef Signal<0xfe6ccd6f> close_t;
	
	// thread safe
	void receive(Message* msg) {
		msg->dst = this;
		engine->receive(msg);
	}
	
	template<typename T>
	void send(T&& msg, Stream* dst) {
		send(&msg, dst, false);
	}
	
	template<typename T, typename R>
	T request(R&& req, Stream* dst) {
		send(&req, dst, false);
		return req.res;
	}
	
	void send(Message* msg, Stream* dst, bool async) {
		msg->src = this;
		engine->send(msg, dst, async);
	}
	
	Message* poll() {
		return poll(2147483647);
	}
	
	Message* poll(int millis) {
		Message* msg = 0;
		if(!queue.pop(msg) && millis >= 0) {
			if(engine->poll(this, millis)) {
				queue.pop(msg);
			}
		}
		return msg;
	}
	
protected:
	uint64_t mac;
	bool open = true;
	
private:
	Engine* engine;
	
	void push(Message* msg) {
		if(open) {
			queue.push(msg);
		} else {
			msg->ack();
		}
		if(msg->mid == close_t::id) {
			open = false;
			queue.push(0);
		}
	}
	
private:
	vnl::util::queue<Message*> queue;
	vnl::util::queue<Fiber*> impl;
	
	friend class Message;
	friend class Object;
	friend class Engine;
	friend class FiberEngine;
	friend class ThreadEngine;
	
};


class Condition : Stream {
public:
	Condition() {}
	
	typedef Signal<0x794f0932> signal_t;
	
	void wait() {
		waiting = true;
		poll()->ack();
		waiting = false;
	}
	
	void notify() {
		if(waiting) {
			send(signal_t(), this);
		}
	}
	
	void set() {
		value = true;
		notify();
	}
	
	void reset() {
		value = false;
	}
	
	void check() {
		while(!value) {
			wait();
		}
	}
	
private:
	bool waiting = false;
	bool value = false;
	
};


}}

#endif /* INCLUDE_PHY_STREAM_H_ */
