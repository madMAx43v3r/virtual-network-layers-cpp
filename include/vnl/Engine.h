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
#include <vnl/Pipe.h>
#include <vnl/Address.h>
#include <vnl/Message.h>
#include <vnl/Memory.h>
#include <vnl/Random.h>
#include <vnl/Queue.h>
#include <vnl/List.h>
#include <vnl/Util.h>


namespace vnl {

class Stream;
class Object;
class ThreadEngine;
class FiberEngine;

// running module in new thread
Address spawn(Object* object, Pipe* pipe = 0);


class Engine : public Node {
public:
	Engine() {
		std::lock_guard<std::mutex> lock(static_mutex);
		instances->push_back(this);
	}
	
	virtual ~Engine() {
		std::lock_guard<std::mutex> lock(static_mutex);
		instances->remove(this);
	}
	
	// thread safe
	virtual void receive(Message* msg) {
		assert(msg->dst || msg->isack);
		msg->gate = this;
		msg->rcv_time = vnl::currentTimeMicros();
		mutex.lock();
		queue.push(msg);
		cond.notify_all();
		if(!msg->isack) {
			num_received++;
		}
		for(auto& func : rcv_hooks) {
			func(msg);
		}
		mutex.unlock();
	}
	
	// thread safe
	void timeout() {
		Queue<Message*> tmp;
		{
			std::lock_guard<std::mutex> lock(mutex);
			int64_t now = vnl::currentTimeMicros();
			int i = 0;
			int count = queue.size();
			Message* msg = 0;
			while(i < count && queue.pop(msg)) {
				if(!msg->isack && !msg->is_no_drop && now - msg->rcv_time > msg->timeout) {
					tmp.push(msg);
				} else {
					queue.push(msg);
				}
				i++;
			}
		}
		Message* msg = 0;
		while(tmp.pop(msg)) {
			msg->is_timeout = true;
			msg->ack();
			num_timeout++;
		}
	}
	
	// all below NOT thread safe
	
	void add_receive_hook(const std::function<void(Message*)>& func) {
		mutex.lock();
		rcv_hooks.push_back(func);
		mutex.unlock();
	}
	
	void send(Message* msg, Basic* dst) {
		msg->src = this;
		msg->dst = dst;
		send_impl(msg, false);
		num_sent++;
	}
	
	void send_async(Message* msg, Basic* dst) {
		msg->src = this;
		msg->dst = dst;
		send_impl(msg, true);
		num_sent++;
	}
	
	virtual bool poll(Stream* stream, int64_t micros) = 0;
	
	virtual void flush() = 0;
	
	int64_t num_sent = 0;
	int64_t num_received = 0;
	int64_t num_timeout = 0;
	int64_t num_cycles = 0;
	
	static List<Engine*>* instances;
	static std::mutex static_mutex;
	
protected:
	void exec(Object* object, Message* init, Pipe* pipe);
	
	Message* collect(int64_t timeout);
	size_t collect(int64_t timeout, vnl::Queue<Message*>& inbox);
	
	virtual void send_impl(Message* msg, bool async) = 0;
	
private:
	std::mutex mutex;
	std::condition_variable cond;
	
	Queue<Message*> queue;
	List<std::function<void(Message*)> > rcv_hooks;
	
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
