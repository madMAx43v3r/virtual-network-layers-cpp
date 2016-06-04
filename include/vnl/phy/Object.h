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

#include "vnl/phy/Engine.h"
#include "vnl/phy/Stream.h"
#include "vnl/phy/Reference.h"
#include "vnl/phy/RingBuffer.h"
#include "vnl/phy/Pool.h"
#include "vnl/phy/Timer.h"
#include "vnl/String.h"
#include "vnl/System.h"


namespace vnl { namespace phy {

class Object : public Node {
public:
	Object();
	Object(uint64_t mac);
	Object(const vnl::String& name);
	
	const vnl::String& getName() const { return my_name; }
	
	// thread safe
	virtual void receive(Message* msg) override {
		stream->receive(msg);
	}
	
	enum {
		ERROR = 1, WARN = 2, INFO = 3
	};
	
	static int global_log_level;
	int log_level = global_log_level;
	
protected:
	virtual ~Object() {}
	
	uint64_t rand() {
		return engine->rand();
	}
	
	void fork(Object* object) {
		engine->fork(object);
	}
	
	void send(Message* msg, Node* dst) {
		stream->send(msg, dst);
	}
	void send_async(Message* msg, Node* dst) {
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
	
	vnl::StringWriter log(int level);
	
	Timer* timeout(int64_t micros, const std::function<void(Timer*)>& func, Timer::type_t type);
	
	void run();
	
	void die();
	
	void exit(Message* msg);
	
	virtual bool handle(Message* msg) { return false; }
	
	virtual void main(Engine* engine) = 0;
	
protected:
	vnl::String my_name;
	
	Region memory;
	MessageBuffer buffer;
	
private:
	void exec(Engine* engine);
	
private:
	Stream* stream = 0;
	Engine* engine = 0;
	
	Timer* timer_begin = 0;
	
	int64_t ref = 0;
	bool dying = false;
	Message* exit_msg = 0;
	
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
Reference<T>::Reference(Engine* engine, const vnl::String& name) 
	:	Reference(engine, vnl::hash64(name))
{
}


}}

#endif /* INCLUDE_PHY_OBJECT_H_ */
