/*
 * Object.h
 *
 *  Created on: Dec 16, 2015
 *      Author: mad
 */

#ifndef INCLUDE_PHY_OBJECT_H_
#define INCLUDE_PHY_OBJECT_H_

#include <functional>

#include "vnl/Engine.h"
#include "vnl/Stream.h"
#include "vnl/Reference.h"
#include "vnl/RingBuffer.h"
#include "vnl/Pool.h"
#include "vnl/Timer.h"
#include "vnl/String.h"


namespace vnl {

class Module : public Basic {
public:
	Module();
	Module(uint64_t mac);
	Module(const char* name);
	Module(const String& name);
	
 	// thread safe
	virtual void receive(Message* msg) override;
	
	enum {
		ERROR = 1, WARN = 2, INFO = 3, DEBUG = 4
	};
	
	static int global_log_level;
	int log_level = global_log_level;
	
	typedef MessageType<String, 0x4f4ab3ef> get_name_t;
	typedef MessageType<int, 0x14a5a142> set_log_level_t;
	
protected:
	virtual ~Module() {}
	
	uint64_t rand() {
		return engine->rand();
	}
	
	void fork(Module* object) {
		engine->fork(object);
	}
	
	void send(Message* msg, Basic* dst) {
		if(dst != this) {
			stream->send(msg, dst);
		} else {
			msg->src = engine;
			process(msg);
		}
	}
	void send_async(Message* msg, Basic* dst) {
		if(dst != this) {
			stream->send_async(msg, dst);
		} else {
			msg->src = engine;
			process(msg);
		}
	}
	
	template<typename T>
	void send(Message* msg, Reference<T>& dst) {
		send(msg, dst.get());
	}
	template<typename T>
	void send_async(Message* msg, Reference<T>& dst) {
		send_async(msg, dst.get());
	}
	
	void flush() {
		stream->flush();
	}
	
	StringWriter log(int level);
	StringWriter log(int level, double time);
	StringWriter log(int level, int64_t time);
	
	Timer* set_timeout(int64_t micros, const std::function<void(Timer*)>& func, Timer::type_t type);
	
	bool poll(int64_t micros);
	
	void run();
	
	void die();
	
	virtual bool handle(Message* msg) { return false; }
	
	virtual void main(Engine* engine) = 0;
	
protected:
	PageAlloc memory;
	MessageBuffer buffer;
	
	StringOutput* log_output = 0;
	
private:
	void exec(Engine* engine);
	
	void process(Message* msg);
	
private:
	String my_name;
	Stream* stream = 0;
	Engine* engine = 0;
	Timer* timer_begin = 0;
	
	int64_t ref = 0;
	bool dying = false;
	
	friend class Engine;
	friend class Registry;
	template<typename T> friend class Reference;
	
};


template<typename T>
Reference<T>::Reference(Engine* engine, T* obj)
	:	mac(obj->getMAC()), engine(engine), obj(obj)
{
	Registry::open_t msg(obj);
	engine->send(&msg, Registry::instance);
}

template<typename T>
Reference<T>::Reference(Engine* engine, uint64_t mac)
	:	mac(mac), engine(engine)
{
}

template<typename T>
Reference<T>::Reference(Engine* engine, const char* name) 
	:	Reference(engine, hash64(name))
{
}

template<typename T>
Reference<T>::Reference(Engine* engine, const String& name) 
	:	Reference(engine, hash64(name))
{
}

template<typename T>
T* Reference<T>::get() {
	if(!obj) {
		Registry::connect_t req(mac);
		engine->send(&req, Registry::instance);
		obj = (T*)req.res;
	}
	return obj;
}

template<typename T>
T* Reference<T>::try_get() {
	if(!obj) {
		Registry::try_connect_t req(mac);
		engine->send(&req, Registry::instance);
		obj = (T*)req.res;
	}
	return obj;
}

template<typename T>
void Reference<T>::close() {
	if(obj) {
		Registry::close_t msg(obj);
		engine->send(&msg, Registry::instance);
		obj = 0;
	}
}



}

#endif /* INCLUDE_PHY_OBJECT_H_ */
