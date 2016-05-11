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
#include "phy/Reference.h"
#include "phy/RingBuffer.h"
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
	
	typedef SignalType<0xfe6ccd6f> delete_t;
	
	// thread safe
	virtual void receive(Message* msg) override {
		stream->receive(msg);
	}
	
protected:
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
	
	void send(Message* msg, Reference& dst) {
		stream->send(msg, dst.get());
	}
	
	void send_async(Message* msg, Reference& dst) {
		stream->send_async(msg, dst.get());
	}
	
	void timeout(int64_t micro, const std::function<void()>& func, bool repeat = false) {
		// TODO
	}
	
	void die();
	
	void run();
	
	virtual bool handle(Message* msg) { return false; }
	
	virtual void main() = 0;
	
	Region memory;
	MessageBuffer buffer;
	Engine* engine = 0;
	
protected:
	typedef MessageType<uint32_t, 0x33145536> timeout_t;
	
	virtual ~Object() {}
	
private:
	typedef SignalType<0x9a4ac2ca> exit_t;
	
	void main(Engine* engine);
	
private:
	vnl::String name;
	Stream* stream = 0;
	
	int64_t ref = 0;
	bool dying = false;
	Message* exit_msg = 0;
	
	friend class Engine;
	friend class Registry;
	
};



}}

#endif /* INCLUDE_PHY_OBJECT_H_ */
