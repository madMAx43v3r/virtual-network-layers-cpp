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
#include "System.h"

namespace vnl { namespace phy {

class Object : protected Stream {
public:
	Object() {
		launch(std::bind(&Object::mainloop, this));
	}
	
	virtual ~Object() {}
	
	// thread safe
	void receive(Message* msg) {
		Stream::receive(msg);
	}
	
	uint64_t getMAC() {
		return mac;
	}
	
	void close() {
		receive(new Stream::close_t(true));
	}
	
protected:
	uint64_t rand() {
		return engine->rand();
	}
	
	void flush() {
		engine->flush();
	}
	
	void yield() {
		int64_t now = System::currentTimeMillis();
		if(now - last_yield >= 10) {
			sleep(0);
			last_yield = now;
		}
	}
	
	void sleep(int millis) {
		Stream stream;
		stream.poll(millis);
	}
	
	taskid_t launch(Runnable* task) {
		return engine->launch(std::bind(&Runnable::run, task));
	}
	
	taskid_t launch(const std::function<void()>& func) {
		return engine->launch(func);
	}
	
	virtual bool handle(Message* msg) {
		return false;
	}
	
private:
	void mainloop() {
		while(true) {
			Message* msg = poll();
			if(!msg) {
				break;
			}
			assert(msg->isack == false);
			if(!handle(msg)) {
				msg->ack();
			}
		}
	}
	
private:
	int64_t last_yield = 0;
	
	friend class Message;
	friend class Stream;
	friend class Engine;
	
};




}}

#endif /* INCLUDE_PHY_OBJECT_H_ */
