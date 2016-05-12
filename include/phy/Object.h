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

#include "phy/Engine.h"
#include "phy/Stream.h"
#include "phy/Router.h"
#include "phy/Reference.h"
#include "phy/RingBuffer.h"
#include "phy/Pool.h"
#include "String.h"
#include "System.h"


namespace vnl { namespace phy {

class Object : public Node {
public:
	Object();
	Object(uint64_t mac);
	Object(const vnl::String& name);
	Object(Object* parent, const vnl::String& name);
	
	const vnl::String& getName() const { return name; }
	
	// thread safe
	virtual void receive(Message* msg) override {
		stream->receive(msg);
	}
	
	typedef SignalType<0xfe6ccd6f> delete_t;
	
protected:
	typedef SignalType<0x9a4ac2ca> exit_t;
	
	virtual ~Object() {}
	
	enum timer_type { REPEAT, MANUAL, ONCE };
	
	class Timer {
	public:
		void reset() {
			active = true;
			deadline = vnl::System::currentTimeMicros() + interval;
		}
		void stop() { active = false; }
		void destroy() { free = true; }
		int64_t now() const { return deadline; }
	private:
		int64_t deadline;
		int64_t interval;
		std::function<void(Timer*)> func;
		timer_type type;
		bool active = true;
		bool free = false;
		Timer* next = 0;
		friend class Object;
	};
	
	uint64_t rand() {
		return engine->rand();
	}
	
	void fork(Object* object) {
		engine->fork(object);
	}
	
	void connect(vnl::Address address) {
		Router::connect_t connect(address);
		send(&connect, Router::instance);
	}
	void close(vnl::Address address) {
		Router::close_t close(address);
		send(&close, Router::instance);
	}
	
	void send(Packet* packet) {
		send(packet, Router::instance);
	}
	void send_async(Packet* packet) {
		send_async(packet, Router::instance);
	}
	
	void send(Message* msg, Node* dst) {
		stream->send(msg, dst);
	}
	void send_async(Message* msg, Node* dst) {
		stream->send_async(msg, dst);
	}
	
	void send(Message* msg, Reference& dst) {
		stream->send(msg, dst.get());
	}
	void send_async(Message* msg, Reference& dst) {
		stream->send_async(msg, dst.get());
	}
	
	Timer* timeout(int64_t micros, const std::function<void(Timer*)>& func, timer_type type);
	
	void run();
	
	void die();
	
	void exit(Message* msg);
	
	virtual bool handle(Message* msg) { return false; }
	
	virtual void main() = 0;
	
	Region memory;
	MessageBuffer buffer;
	Engine* engine = 0;
	
private:
	void main(Engine* engine);
	
private:
	vnl::String name;
	Stream* stream = 0;
	
	Timer* timer_begin = 0;
	Pool<Timer> timers;
	
	int64_t ref = 0;
	bool dying = false;
	Message* exit_msg = 0;
	
	friend class Engine;
	friend class Registry;
	
};



}}

#endif /* INCLUDE_PHY_OBJECT_H_ */
