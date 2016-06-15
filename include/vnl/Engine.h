/*
 * Engine.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ENGINE_H_
#define INCLUDE_PHY_ENGINE_H_

#include <unordered_set>
#include <mutex>
#include <condition_variable>

#include "vnl/Message.h"
#include "vnl/Base.h"
#include "vnl/Memory.h"
#include "vnl/RingBuffer.h"
#include "vnl/Random.h"
#include "vnl/Queue.h"
#include "vnl/Util.h"


namespace vnl {

class Stream;
class Module;
class ThreadEngine;
class FiberEngine;
template<typename T> class Reference;


class Engine : public Base {
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
	
protected:
	bool dorun = true;
	
	void exec(Module* object);
	
	uint64_t rand() {
		return generator.rand();
	}
	
	void send(Base* src, Message* msg, Base* dst) {
		send_impl(src, msg, dst, false);
	}
	
	void send_async(Base* src, Message* msg, Base* dst) {
		send_impl(src, msg, dst, true);
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
	
	virtual void send_impl(Base* src, Message* msg, Base* dst, bool async) = 0;
	
	virtual bool poll(Stream* stream, int64_t micros) = 0;
	
	virtual void flush() = 0;
	
	virtual void fork(Module* object) = 0;
	
protected:
	Region memory;
	MessageBuffer buffer;
	
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




}

#endif /* INCLUDE_PHY_ENGINE_H_ */
