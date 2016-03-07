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

class Object : public Stream {
public:
	Object() {
		engine->launch(std::bind(&Object::mainloop, this), 0);
	}
	
	virtual ~Object() {}
	
	uint64_t getMAC() {
		return mac;
	}
	
	typedef Signal<0xfe6ccd6f> close_t;
	
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
	
	taskid_t launch(const std::function<void()>& func) {
		taskid_t task = engine->launch(func, this);
		tasks[task.id] = task;
		return task;
	}
	
	void wait_on(const taskid_t& task, Stream* stream) {
		engine->wait_on(task, stream);
	}
	
	virtual bool handle(Message* msg) {
		return false;
	}
	
private:
	void mainloop();
	
private:
	std::unordered_map<uint64_t, taskid_t> tasks;
	int64_t last_yield = 0;
	
	friend class Message;
	friend class Stream;
	friend class Engine;
	
};



}}

#endif /* INCLUDE_PHY_OBJECT_H_ */
