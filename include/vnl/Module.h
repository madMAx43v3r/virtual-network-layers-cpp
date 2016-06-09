/*
 * Object.h
 *
 *  Created on: Dec 16, 2015
 *      Author: mad
 */

#ifndef INCLUDE_PHY_OBJECT_H_
#define INCLUDE_PHY_OBJECT_H_

#include <functional>
#include <unordered_map>

#include "vnl/Engine.h"
#include "vnl/Stream.h"
#include "vnl/Reference.h"
#include "vnl/RingBuffer.h"
#include "vnl/Pool.h"
#include "vnl/Timer.h"
#include "vnl/String.h"


namespace vnl {

class Module : public Base {
public:
	Module();
	Module(uint64_t mac);
	Module(const char* name);
	Module(const String& name);
	
	const String& getName() const { return my_name; }
	
	// thread safe
	virtual void receive(Message* msg) override {
		stream->receive(msg);
	}
	
	enum {
		ERROR = 1, WARN = 2, INFO = 3, DEBUG = 4
	};
	
	static int global_log_level;
	int log_level = global_log_level;
	
protected:
	virtual ~Module() {}
	
	uint64_t rand() {
		return engine->rand();
	}
	
	void fork(Module* object) {
		engine->fork(object);
	}
	
	void send(Message* msg, Base* dst) {
		stream->send(msg, dst);
	}
	void send_async(Message* msg, Base* dst) {
		stream->send_async(msg, dst);
	}
	
	template<typename T>
	void send(Message* msg, Reference<T>& dst) {
		stream->send(msg, dst.get());
	}
	template<typename T>
	void send_async(Message* msg, Reference<T>& dst) {
		stream->send_async(msg, dst.get());
	}
	
	StringWriter log(int level);
	
	Timer* timeout(int64_t micros, const std::function<void(Timer*)>& func, Timer::type_t type);
	
	bool poll(int64_t micros);
	
	void run();
	
	void die();
	
	virtual bool handle(Message* msg) { return false; }
	
	virtual void main(Engine* engine) = 0;
	
protected:
	Region memory;
	MessageBuffer buffer;
	
private:
	void exec(Engine* engine);
	
private:
	String my_name;
	Stream* stream = 0;
	Engine* engine = 0;
	
	Timer* timer_begin = 0;
	
	int64_t ref = 0;
	bool dying = false;
	
	friend class Engine;
	friend class Registry;
	
};


template<typename T>
Reference<T>::Reference(Engine* engine, T* obj)
	:	mac(obj->getMAC()), engine(engine), obj(obj)
{
	Registry::open_t msg(obj);
	engine->send(engine, &msg, Registry::instance);
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


}

#endif /* INCLUDE_PHY_OBJECT_H_ */
