/*
 * Stream.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_STREAM_H_
#define INCLUDE_PHY_STREAM_H_

#include "util/simple_queue.h"
#include "phy/Message.h"

namespace vnl { namespace phy {

class Engine;
class Object;
class Fiber;

class Stream {
public:
	Stream(Object* object);
	Stream(Object* object, uint64_t sid);
	~Stream();
	
	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;
	
	// thread safe
	void receive(Message* msg);
	
	template<typename T>
	void send(T&& msg, Object* dst);
	
	void send(Message* msg, Object* dst, bool async = false);
	
	Message* poll();
	Message* poll(int millis);
	
	template<typename T>
	T read();
	
	template<typename T, typename R>
	T request(R&& req, Object* dst);
	
	Object* obj;
	uint64_t sid;
	vnl::util::simple_queue<Fiber*> impl;
	
private:
	void push(Message* msg) {
		queue.push(msg);
	}
	
	vnl::util::simple_queue<Message*> queue;
	
	friend class Object;
	friend class Engine;
	
};


class Condition : Stream {
public:
	Condition(Object* object) : Stream(object) {}
	
	typedef Signal<0x794f0932> signal_t;
	
	void wait() {
		waiting = true;
		read<signal_t>();
		waiting = false;
	}
	
	void notify() {
		if(waiting) {
			send(signal_t(), obj);
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


template<typename T, typename R>
T Stream::request(R&& req, Object* dst) {
	send(req, dst);
	return req.res;
}


}}

#endif /* INCLUDE_PHY_STREAM_H_ */
