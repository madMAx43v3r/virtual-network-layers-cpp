/*
 * Engine.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ENGINE_H_
#define INCLUDE_PHY_ENGINE_H_

#include <mutex>
#include <condition_variable>

#include <vnl/Basic.h>
#include <vnl/Address.h>
#include <vnl/Message.h>
#include <vnl/Memory.h>
#include <vnl/Random.h>
#include <vnl/Queue.h>
#include <vnl/Util.h>


namespace vnl {

class Stream;
class Object;
class ThreadEngine;
class FiberEngine;

// running module in new thread
Address spawn(Object* object);

// running module in a fiber if possible, otherwise uses spawn
Address fork(Object* object);


class Engine : public Basic {
public:
	Engine();
	
	virtual ~Engine() {}
	
	// thread safe
	virtual void receive(Message* msg) {
		msg->gate = this;
		mutex.lock();
		queue.push(msg);
		num_queued++;
		cond.notify_all();
		mutex.unlock();
	}
	
	// all below NOT thread safe
	
	void send(Message* msg, Basic* dst) {
		send_impl(msg, dst, false);
	}
	
	void send_async(Message* msg, Basic* dst) {
		send_impl(msg, dst, true);
	}
	
	virtual void fork(Object* object) = 0;
	
	virtual bool poll(Stream* stream, int64_t micros) = 0;
	
	virtual void flush() = 0;
	
protected:
	void exec(Object* object, Message* init);
	
	Message* collect(int64_t timeout);
	size_t collect(int64_t timeout, vnl::Queue<Message*>& inbox);
	
	virtual void send_impl(Message* msg, Basic* dst, bool async) = 0;
	
private:
	std::mutex mutex;
	std::condition_variable cond;
	
	Queue<Message*> queue;
	size_t num_queued;
	
	friend class Stream;
	friend class Object;
	friend class ThreadEngine;
	friend class FiberEngine;
	
};


class GlobalLogWriter : public StringOutput {
public:
	GlobalLogWriter(Object* node) : node(node) {}
	virtual void write(const String& str);
	int level = 0;
private:
	Object* node;
};



}

#endif /* INCLUDE_PHY_ENGINE_H_ */
