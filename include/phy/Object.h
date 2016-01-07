/*
 * Object.h
 *
 *  Created on: Dec 16, 2015
 *      Author: mad
 */

#ifndef INCLUDE_OBJECT_H_
#define INCLUDE_OBJECT_H_

#include <stdint.h>
#include <random>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <atomic>

#include "System.h"
#include "Runnable.h"
#include "Util.h"
#include "util/simple_hashmap.h"

namespace vnl { namespace phy {

static const int FOREVER = 2147483647;

class Object;
class Stream;
class Engine;

class Message {
public:
	Message() {}
	virtual ~Message() {}
	Message(uint32_t type, bool async = false) : type(type), async(async) {}
	Message(Object* dst, uint32_t type) : type(type), dst(dst) {}
	Message(Object* dst, uint64_t sid, uint32_t type) : type(type), dst(dst), sid(sid) {}
	template<typename T> T* cast();
	void ack();
	virtual std::string toString();
	uint32_t type = 0;
	Object* src = 0;
	Object* dst = 0;
	uint64_t sid = 0;
	void* impl = 0;
	bool isack = false;
	bool async = false;
};

template<typename T, uint32_t N>
class Generic : public Message {
public:
	Generic() : Message() {}
	Generic(const T& obj, bool async = false) : Message(N, async), data(obj) {}
	Generic(Object* dst, const T& obj) : Message(dst, N), data(obj) {}
	Generic(Object* dst, uint64_t sid, const T& obj) : Message(dst, sid, N), data(obj) {}
	static const uint32_t id = N;
	T data;
};

template<typename T, typename P, uint32_t N>
class Request : public Message {
public:
	Request(const P& args, bool async = false) : Message(N, async), args(args) {}
	Request(Object* dst, const P& args) : Message(dst, N), args(args) {}
	void ack(const T& result) {
		res = result;
		Message::ack();
	}
	static const uint32_t id = N;
	typedef T res_t;
	typedef P args_t;
	T res;
	P args;
};

class Stream {
public:
	Stream(Object* object);
	Stream(Object* object, uint64_t sid);
	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;
	~Stream();
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
	friend class Engine;
};

template<typename T, int N>
class SendBuffer {
public:
	SendBuffer(Object* obj) : obj(obj), left(N) {}
	~SendBuffer();
	T* get(T&& msg);
private:
	Object* obj;
	int left;
	T buf[N];
};

class Engine {
public:
	Engine() {
		static std::atomic<int> counter;
		generator.seed(Util::hash64(counter++, System::nanoTime()));
	}
	virtual ~Engine() {}
	virtual void send(Message* msg) = 0;
	virtual void flush() = 0;
	virtual void handle(Message* msg) = 0;
	virtual void open(Stream* stream) = 0;
	virtual void close(Stream* stream) = 0;
	virtual bool poll(Stream* stream, int millis) = 0;
	virtual uint64_t launch(Runnable* task) = 0;
	virtual void cancel(uint64_t tid) = 0;
	virtual int timeout() = 0;
	uint64_t rand() {
		return Util::hash64(generator());
	}
protected:
	void forward(Message* msg);
	Stream* get_stream(Object* obj, uint64_t sid);
	void dispatch(Message* msg);
	void dispatch(Message* msg, Stream* stream);
private:
	std::default_random_engine generator;
	
};

class Object {
public:
	Object(Object* parent) : link(parent->link), engine(parent->engine) {
		mac = rand();
	}
	Object(Engine* engine) : link(this), engine(engine) {
		mac = rand();
	}
	virtual ~Object() {
		for(uint64_t tid : tasks) {
			engine->cancel(tid);
		}
	}
	
	void receive(Message* msg) {
		msg->dst = this;
		link->receive(msg, 0);
	}
	
	Object* getLink() {
		return link;
	}
	
	uint64_t getMAC() {
		return mac;
	}
	
	virtual void receive(Message* msg, Object* src) {
		if(src == link) {
			engine->handle(msg);
		} else {
			link->receive(msg, src);
		}
	}
	
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
		streams.put(stream->sid, stream);
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
		uint64_t tid = engine->launch(task);
		tasks.push(tid);
		return tid;
	}
	
	uint64_t launch(const std::function<void()>& func) {
		return launch(new Bind(func));
	}
	
	void cancel(uint64_t tid) {
		engine->cancel(tid);
		tasks.erase(tid);
	}
	
	virtual void handle(Message* msg) {
		msg->ack();
	}
	
private:
	Stream* get_stream(uint64_t sid) {
		Stream** stream = streams.get(sid);
		if(stream) {
			return *stream;
		}
		return 0;
	}
	
protected:
	Engine* engine;
	
private:
	Object* link;
	uint64_t mac;
	int yield_counter = 0;
	
	vnl::util::simple_hashmap<uint64_t, Stream*> streams;
	vnl::util::simple_queue<uint64_t> tasks;
	
	friend class Message;
	friend class Stream;
	friend class Engine;
	template<typename M, int N>
	friend class SendBuffer;
	
};

template<typename T, int N>
SendBuffer<T,N>::~SendBuffer() {
	obj->flush();
}

template<typename T, int N>
T* SendBuffer<T,N>::get(T&& msg) {
	if(!left) {
		obj->flush();
		left = N;
	}
	buf[N - left] = msg;
	return &buf[N - left--];
}

template<typename T>
T* Message::cast() {
	if(type == T::id) {
		return (T*)this;
	} else {
		return 0;
	}
}

void Message::ack() {
	if(!isack) {
		isack = true;
		if(src) {
			src->receive(this, dst);
		} else if(async) {
			delete this;
		}
	}
}

std::string Message::toString() {
	std::ostringstream ss;
	ss << "[" << Util::demangle(this) << "] type=0x" << std::hex << type << std::dec
			<< " src=" << src << " dst=" << dst << " sid=" << sid << " isack=" << isack << " async=" << async;
	return ss.str();
}

Stream::Stream(Object* object) : Stream(object, object->rand()) {}

Stream::Stream(Object*object , uint64_t sid) : obj(object), sid(sid) {
	obj->open(this);
}

Stream::~Stream() {
	obj->close(this);
	for(Message* msg : queue) {
		msg->ack();
	}
}

template<typename T>
void Stream::send(T&& msg) {
	msg.sid = sid;
	obj->send(msg);
}

void Stream::send(Message* msg, bool async) {
	msg->sid = sid;
	obj->send(msg, async);
}

void Stream::receive(Message* msg) {
	msg->sid = sid;
	obj->receive(msg);
}

void Stream::receive(Message* msg, Object* src) {
	if(src == obj) {
		queue.push(msg);
	} else {
		obj->receive(msg, src);
	}
}

Message* Stream::poll() {
	return poll(FOREVER);
}

Message* Stream::poll(int millis) {
	Message* msg = 0;
	if(!queue.pop(msg) && millis >= 0) {
		if(obj->engine->poll(this, millis)) {
			queue.pop(msg);
		}
	}
	return msg;
}

template<typename T>
T Stream::read() {
	Message* msg = poll();
	if(msg->type == T::id) {
		T res = *((T*)msg);
		msg->ack();
		return res;
	} else {
		msg->ack();
		return T();
	}
}

template<typename T, typename R>
T Stream::request(R&& req) {
	send(req);
	return req.res;
}

void Engine::forward(Message* msg) {
	msg->dst->receive(msg, msg->src);
}

Stream* Engine::get_stream(Object* obj, uint64_t sid) {
	return obj->get_stream(sid);
}

void Engine::dispatch(Message* msg) {
	msg->dst->handle(msg);
}

void Engine::dispatch(Message* msg, Stream* stream) {
	stream->receive(msg, msg->dst);
}


}}

#endif /* INCLUDE_LINK_H_ */
