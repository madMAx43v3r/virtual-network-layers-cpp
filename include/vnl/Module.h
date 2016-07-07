/*
 * Object.h
 *
 *  Created on: Dec 16, 2015
 *      Author: mad
 */

#ifndef INCLUDE_PHY_OBJECT_H_
#define INCLUDE_PHY_OBJECT_H_

#include <vnl/Engine.h>
#include <vnl/Stream.h>
#include <vnl/Reference.h>
#include <vnl/RingBuffer.h>
#include <vnl/Pool.h>
#include <vnl/Timer.h>
#include <vnl/String.h>
#include <vnl/Packet.h>


namespace vnl {

class Module : public Basic {
public:
	Module();
	Module(uint64_t mac);
	Module(const char* name);
	Module(const String& name);
	
 	// thread safe
	virtual void receive(Message* msg);
	
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
	
	Module* fork(Module* object) {
		engine->fork(object);
		return object;
	}
	
	void open(Address address) {
		stream.open(address);
	}
	void close(Address address) {
		stream.close(address);
	}
	
	void send(Message* msg, Basic* dst) {
		if(dst != this) {
			stream.send(msg, dst);
		} else {
			msg->src = engine;
			process(msg);
		}
	}
	void send_async(Message* msg, Basic* dst) {
		if(dst != this) {
			stream.send_async(msg, dst);
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
	
	void send(Packet* packet, Address dst) {
		stream.send(packet, dst);
	}
	void send_async(Packet* packet, Address dst) {
		stream.send_async(packet, dst);
	}
	
	void flush() {
		stream.flush();
	}
	
	StringWriter log(int level);
	StringWriter log(int level, double time);
	StringWriter log(int level, int64_t time);
	
	Timer* set_timeout(int64_t micros, const std::function<void(Timer*)>& func, Timer::type_t type);
	
	bool poll(int64_t micros);
	
	bool sleep(int64_t secs);
	bool usleep(int64_t micros);
	
	void die();
	
	virtual void run();
	
	virtual bool handle(Message* msg) {
		if(msg->msg_id == Packet::MID) {
			Packet* pkt = (Packet*)msg;
			return handle(pkt);
		}
		return false;
	}
	
	virtual bool handle(Packet* pkt) { return false; }
	
	virtual void main(Engine* engine) = 0;
	
protected:
	PageAlloc memory;
	MessageBuffer buffer;
	
private:
	void exec(Engine* engine);
	
	void process(Message* msg);
	
private:
	String my_name;
	Stream stream;
	Engine* engine = 0;
	Timer* timer_begin = 0;
	GlobalLogWriter log_writer;
	
	int64_t ref = 0;
	bool dying = false;
	
	friend class Engine;
	friend class Registry;
	friend class Receiver;
	friend class GlobalLogWriter;
	template<typename T> friend class Reference;
	
};


class Receiver {
public:
	Receiver(Stream* node, Address addr) : node(node), address(addr) {
		node->open(address);
	}
	
	Receiver(Module* node, Address addr) : Receiver(&node->stream, addr) {}
	
	Receiver(const Receiver&) = delete;
	Receiver& operator=(const Receiver&) = delete;
	
	~Receiver() {
		node->close(address);
	}
	
private:
	Stream* node;
	Address address;
	
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
