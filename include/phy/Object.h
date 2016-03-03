/*
 * Object.h
 *
 *  Created on: Dec 16, 2015
 *      Author: mad
 */

#ifndef INCLUDE_PHY_OBJECT_H_
#define INCLUDE_PHY_OBJECT_H_

#include <unordered_map>
#include <vector>

#include "phy/Message.h"
#include "phy/Stream.h"
#include "Runnable.h"
#include "System.h"
#include "Util.h"

namespace vnl { namespace phy {

class Engine;
class Fiber;

struct taskid_t {
	uint32_t id;
	Fiber* impl;
	taskid_t() : id(0), impl(0) {}
};

class Object {
public:
	Object();
	virtual ~Object() {}
	
	// thread safe
	void receive(Message* msg);
	
	uint64_t mac;
	
protected:
	uint64_t rand();
	
	template<typename T, typename R>
	T request(R&& req, Object* dst) {
		send(req, dst);
		return req.res;
	}
	
	template<typename T>
	void send(T&& msg, Object* dst) {
		send(&msg, dst, false);
	}
	
	void send(Message* msg, Object* dst, bool async);
	
	void flush();
	
	void open(Stream* stream);
	void close(Stream* stream);
	
	void yield();
	void sleep(int millis);
	
	taskid_t launch(Runnable* task);
	taskid_t launch(const std::function<void()>& func);
	
	void cancel(taskid_t task);
	
	virtual bool handle(Message* msg) {
		return false;
	}
	
	void exit() {
		delete this;
	}
	
private:
	void mainloop();
	
	void process() {
		if(!running) {
			launch(main);
		}
	}
	
	Stream* get_stream(uint64_t sid) {
		auto iter = streams.find(sid);
		if(iter != streams.end()) {
			return iter->second;
		}
		return &queue;
	}
	
private:
	Engine* engine;
	
	std::unordered_map<uint64_t, Stream*> streams;
	Stream queue;
	
	std::function<void()> main;
	bool running;
	
	int64_t last_yield = 0;
	
	friend class Message;
	friend class Stream;
	friend class Engine;
	
};


template<typename T>
void Stream::send(T&& msg, Object* dst) {
	msg.sid = sid;
	obj->send(msg, dst);
}



}}

#endif /* INCLUDE_PHY_OBJECT_H_ */
