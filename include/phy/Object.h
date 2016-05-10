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
	
	typedef Signal<0xfe6ccd6f> delete_t;
	
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
	
	template<typename T>
	void send(T&& msg, Node* dst) {
		stream->send(msg, dst);
	}
	
	template<typename T>
	void send_async(T&& msg, Node* dst) {
		stream->send_async(msg, dst);
	}
	
	template<typename T, typename R>
	T request(R&& req, Node* dst) {
		return stream->request<T>(req, dst);
	}
	
	void send(Message* msg, Node* dst) {
		stream->send(msg, dst);
	}
	
	void send_async(Message* msg, Node* dst) {
		stream->send_async(msg, dst);
	}
	
	template<typename T>
	void send(T&& msg, Reference& dst) {
		stream->send(msg, dst.get());
	}
	
	template<typename T>
	void send_async(T&& msg, Reference& dst) {
		stream->send_async(msg, dst.get());
	}
	
	template<typename T, typename R>
	T request(R&& req, Reference& dst) {
		return stream->request<T>(req, dst.get());
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
	
	virtual bool handle(Message* msg) {
		return false;
	}
	
	virtual bool startup() { return true; }
	
	virtual void shutdown() {}
	
	Region memory;
	Engine* engine = 0;
	
protected:
	typedef Generic<uint32_t, 0x33145536> timeout_t;
	
	virtual ~Object() {
		std::cout << "Object " << this << " being deleted" << std::endl;
	}
	
private:
	typedef Signal<0x9a4ac2ca> exit_t;
	
	void run(Engine* engine);
	
private:
	vnl::String name;
	Stream* stream = 0;
	
	int64_t ref = 0;
	bool dying = false;
	
	friend class Engine;
	friend class Registry;
	
};



}}

#endif /* INCLUDE_PHY_OBJECT_H_ */
