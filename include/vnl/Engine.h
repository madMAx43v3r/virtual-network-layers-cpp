/*
 * Engine.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ENGINE_H_
#define INCLUDE_PHY_ENGINE_H_

#include <vnl/Basic.h>
#include <unordered_set>
#include <mutex>
#include <condition_variable>

#include "vnl/Message.h"
#include "vnl/Memory.h"
#include "vnl/Random.h"
#include "vnl/Queue.h"
#include "vnl/Util.h"


namespace vnl {

class Stream;
class Module;
class ThreadEngine;
class FiberEngine;
template<typename T> class Reference;

// running module in new thread
void spawn(Module* object);

// running module in a fiber if possible, otherwise uses spawn
void fork(Module* object);


class Engine : public Basic {
public:
	Engine();
	Engine(const std::string& name);
	virtual ~Engine() {}
	
	// thread safe
	virtual void receive(Message* msg) override {
		if(!msg->dst || msg->dst == this) {
			msg->ack();
		} else {
			msg->gate = this;
			mutex.lock();
			queue.push(msg);
			cond.notify_all();
			mutex.unlock();
		}
	}
	
	// not thread safe
	virtual void fork(Module* object) = 0;
	
protected:
	void exec(Module* object);
	
	uint64_t rand() {
		return generator.rand();
	}
	
	void send(Message* msg, Basic* dst) {
		send_impl(msg, dst, false);
	}
	
	void send_async(Message* msg, Basic* dst) {
		send_impl(msg, dst, true);
	}
	
	Message* collect(int64_t timeout) {
		std::unique_lock<std::mutex> ulock(mutex);
		Message* msg = 0;
		if(queue.pop(msg)) {
			return msg;
		}
		if(timeout != 0) {
			if(timeout > 0) {
				cond.wait_for(ulock, std::chrono::microseconds(timeout));
			} else {
				cond.wait(ulock);
			}
			queue.pop(msg);
		}
		return msg;
	}
	
	size_t collect(int64_t timeout, vnl::Queue<Message*>& inbox) {
		std::unique_lock<std::mutex> ulock(mutex);
		size_t count = 0;
		Message* msg = 0;
		while(queue.pop(msg)) {
			inbox.push(msg);
			count++;
		}
		if(!count && timeout != 0) {
			if(timeout > 0) {
				cond.wait_for(ulock, std::chrono::microseconds(timeout));
			} else {
				cond.wait(ulock);
			}
			while(queue.pop(msg)) {
				inbox.push(msg);
				count++;
			}
		}
		return count;
	}
	
	virtual void send_impl(Message* msg, Basic* dst, bool async) = 0;
	
	virtual bool poll(Stream* stream, int64_t micros) = 0;
	
	virtual void flush() = 0;
	
private:
	std::mutex mutex;
	std::condition_variable cond;
	
	Queue<Message*> queue;
	
	Random64 generator;
	
	friend class Stream;
	friend class Module;
	friend class ThreadEngine;
	friend class FiberEngine;
	template<typename T> friend class Reference;
	
};


class GlobalLogWriter : public StringOutput {
public:
	GlobalLogWriter(Module* node) : node(node) {}
	virtual void write(const String& str);
private:
	Module* node;
};



}

#endif /* INCLUDE_PHY_ENGINE_H_ */
