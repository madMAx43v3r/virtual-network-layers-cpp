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
#include <assert.h>

#include "phy/Engine.h"
#include "phy/Stream.h"
#include "phy/Reference.h"
#include "System.h"

namespace vnl { namespace phy {

class Object : Node {
public:
	Object();
	Object(uint64_t mac);
	Object(const std::string& name);
	Object(Object* parent, const std::string& name);
	
	uint64_t getMAC() const { return mac; }
	const std::string& getName() const { return name; }
	
	// thread safe
	virtual void receive(Message* msg) override {
		stream->receive(msg);
	}
	
protected:
	uint64_t rand() {
		return stream->rand();
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
	
	void die();
	
	virtual bool handle(Message* msg) {
		return false;
	}
	
	virtual void shutdown() {}
	
private:
	typedef Signal<0x9a4ac2ca> exit_t;
	
	virtual ~Object() {}
	
	void mainloop(Engine* engine);
	
private:
	uint64_t mac;
	std::string name;
	Stream* stream = 0;
	
	int64_t ref = 0;
	bool dying = false;
	
	friend class Registry;
	friend class Engine;
	
};



}}

#endif /* INCLUDE_PHY_OBJECT_H_ */
