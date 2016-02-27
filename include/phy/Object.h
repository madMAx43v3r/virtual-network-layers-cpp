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

#include "Message.h"
#include "Stream.h"
#include "Engine.h"
#include "Runnable.h"
#include "Util.h"

namespace vnl { namespace phy {

class Object : Runnable {
public:
	Object();
	virtual ~Object() {}
	
	void receive(Message* msg) {
		msg->dst = this;
		engine->receive(msg, 0);
	}
	
	uint64_t mac;
	
protected:
	uint64_t rand() {
		return engine->rand();
	}
	
	template<typename T, typename R>
	T request(R&& req) {
		send(req);
		return req.res;
	}
	
	template<typename T>
	void send(T&& msg) {
		send(&msg, false);
	}
	
	void send(Message* msg, bool async = false) {
		msg->seq = seq_counter++;
		msg->async = async;
		if(msg->dst == this) {
			msg->src = 0;
			handle(msg);
		} else {
			msg->src = this;
			msg->dst->receive(msg, this);
			engine->send(msg);
		}
	}
	
	void open(Stream* stream) {
		streams[stream->sid] = stream;
	}
	
	void close(Stream* stream) {
		streams.erase(stream->sid);
	}
	
	void flush() {
		engine->flush();
	}
	
	void yield() {
		int now = System::currentTimeMillis();
		if(now - last_yield >= 10) {
			sleep(0);
			last_yield = now;
		}
	}
	
	void sleep(int millis) {
		Stream stream(this);
		stream.poll(millis);
	}
	
	void* launch(Runnable* task) {
		return engine->launch(task);
	}
	
	void* launch(const std::function<void()>& func) {
		return launch(new Bind(func));
	}
	
	void cancel(void* task) {
		engine->cancel(task);
	}
	
	virtual void run();
	
	virtual bool handle(Message* msg) {
		return false;
	}
	
private:
	Object(Engine* engine);
	
	virtual void receive(Message* msg, Object* src);
	
	Stream* get_stream(uint64_t sid);
	
protected:
	Engine* engine;
	
private:
	Stream queue;
	void* task;
	
	std::unordered_map<uint64_t, Stream*> streams;
	int seq_counter = 0;
	int last_yield = 0;
	
	friend class Message;
	friend class Stream;
	friend class Engine;
	
};




}}

#endif /* INCLUDE_PHY_OBJECT_H_ */
