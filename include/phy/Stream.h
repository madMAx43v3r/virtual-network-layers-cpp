/*
 * Stream.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_STREAM_H_
#define INCLUDE_PHY_STREAM_H_

#include "Message.h"
#include "util/simple_queue.h"

namespace vnl { namespace phy {

class Engine;
class Object;

class Stream {
public:
	Stream(Object* object);
	Stream(Object* object, uint64_t sid);
	~Stream();
	
	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;
	
	template<typename T>
	void send(T&& msg);
	
	void send(Message* msg, bool async = false);
	
	void receive(Message* msg);
	
	Message* poll();
	Message* poll(int millis);
	
	template<typename T>
	T read();
	
	template<typename T, typename R>
	T request(R&& req);
	
	Object* obj;
	uint64_t sid;
	
private:
	void receive(Message* msg, Object* src);
	
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
			send(signal_t(obj));
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


template<typename T>
void Stream::send(T&& msg) {
	msg.sid = sid;
	obj->send(msg);
}

template<typename T, typename R>
T Stream::request(R&& req) {
	send(req);
	return req.res;
}


}}

#endif /* INCLUDE_PHY_STREAM_H_ */
