/*
 * Stream.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_STREAM_H_
#define INCLUDE_PHY_STREAM_H_

#include "phy/Engine.h"
#include "phy/Memory.h"
#include "phy/Queue.h"

namespace vnl { namespace phy {

class FiberEngine;
class ThreadEngine;


class Stream {
public:
	Stream() : engine(Engine::local) {
		mac = engine->rand();
	}
	
	Stream(uint64_t sid) : engine(Engine::local), mac(sid) {}
	
	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;
	
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
	
	void close() {
		Message* msg;
		while(queue.pop(msg)) {
			msg->ack();
		}
	}
	
	uint64_t mac;
	
private:
	Engine* engine;
	
	void push(Message* msg) {
		queue.push(msg);
	}
	
	Queue<Message*> queue;
	Queue<Fiber*> impl;
	
	friend class Message;
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
