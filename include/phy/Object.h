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

class Link;

template<typename M, int N>
class SendBuffer;

class Object {
public:
	Object();
	virtual ~Object() {}
	
	void receive(Message* msg) {
		msg->dst = this;
		link->receive(msg, 0);
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
		msg->src = this;
		msg->async = async;
		engine->send(msg);
	}
	
	void open(Stream* stream) {
		streams[stream->sid] = stream;
		engine->open(stream);
	}
	
	void close(Stream* stream) {
		engine->close(stream);
		streams.erase(stream->sid);
	}
	
	void flush() {
		engine->flush();
	}
	
	void yield(int mod = 100) {
		yield_counter++;
		if(yield_counter >= mod) {
			sleep(0);
			yield_counter = 0;
		}
	}
	
	void sleep(int millis) {
		Stream s(this);
		s.poll(millis);
	}
	
	uint64_t launch(Runnable* task) {
		return engine->launch(task);
	}
	
	uint64_t launch(const std::function<void()>& func) {
		return launch(new Bind(func));
	}
	
	void cancel(uint64_t tid) {
		engine->cancel(tid);
	}
	
	virtual void handle(Message* msg) {}
	
private:
	Object(Engine* engine);
	
	virtual void receive(Message* msg, Object* src) {
		if(src == link) {
			engine->handle(msg);
		} else {
			link->receive(msg, src);
		}
	}
	
	Stream* get_stream(uint32_t sid) {
		auto iter = streams.find(sid);
		if(iter != streams.end()) {
			return iter->second;
		}
		return 0;
	}
	
protected:
	Engine* engine;
	
private:
	Object* link;
	std::unordered_map<uint64_t, Stream*> streams;
	int yield_counter = 0;
	
	friend class Message;
	friend class Stream;
	friend class Engine;
	friend class Link;
	template<typename M, int N>
	friend class SendBuffer;
	
};


template<typename T, int N>
class SendBuffer {
public:
	SendBuffer(Object* obj) : obj(obj), left(N) {}
	
	~SendBuffer() {
		obj->flush();
	}
	
	T* get() {
		return put(T());
	}
	
	T* put(T&& msg) {
		if(!left) {
			obj->flush();
			left = N;
		}
		buf[N - left] = msg;
		return &buf[N - left--];
	}
	
private:
	Object* obj;
	int left;
	T buf[N];
	
};


}}

#endif /* INCLUDE_PHY_OBJECT_H_ */
