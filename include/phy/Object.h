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

class Object : public Stream {
public:
	Object();
	Object(const std::string& name);
	Object(Object* parent, const std::string& name);
	
	uint64_t getMAC() {
		return mac;
	}
	
	typedef Signal<0xfe6ccd6f> delete_t;
	
protected:
	uint64_t rand() {
		return engine->rand();
	}
	
	template<typename T>
	void send(T&& msg, Reference& dst) {
		Stream::send(msg, dst.get());
	}
	
	template<typename T, typename R>
	T request(R&& req, Reference& dst) {
		return Stream::request<T>(req, dst.get());
	}
	
	void send(Message* msg, Reference& dst, bool async) {
		Stream::send(msg, dst.get(), async);
	}
	
	void flush() {
		engine->flush();
	}
	
	void die();
	
	virtual bool handle(Message* msg) {
		return false;
	}
	
	virtual void shutdown() {}
	
private:
	typedef Signal<0x9a4ac2ca> exit_t;
	
	Object(uint64_t mac);
	
	virtual ~Object() {}
	
	void bind();
	
	void mainloop();
	
private:
	std::string name;
	uint64_t mac;
	taskid_t task;
	
	friend class Message;
	friend class Stream;
	friend class Engine;
	friend class Registry;
	
};



}}

#endif /* INCLUDE_PHY_OBJECT_H_ */
