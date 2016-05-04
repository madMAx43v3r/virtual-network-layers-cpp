/*
 * Engine.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ENGINE_H_
#define INCLUDE_PHY_ENGINE_H_

#include <random>
#include <atomic>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <assert.h>

#include "util/spinlock.h"
#include "phy/Message.h"
#include "phy/Node.h"
#include "phy/Queue.h"
#include "phy/RingBuffer.h"
#include "System.h"
#include "Util.h"


namespace vnl { namespace phy {

class Page;
class Fiber;
class Stream;
class Object;

class Engine : public Node {
public:
	Engine();
	Engine(uint64_t mac);
	Engine(const std::string& name);
	virtual ~Engine();
	
	static thread_local Engine* local;
	
	// thread safe
	virtual void receive(Message* msg) override {
		msg->safe = true;
		sync.lock();
		queue.push(msg);
		sync.unlock();
		if(waiting-- == 1) {
			lock();
			notify();
			unlock();
		}
	}
	
	virtual void run(Object* object) = 0;
	
protected:
	bool dorun = true;
	
	uint64_t rand() {
		return generator();
	}
	
	template<typename T>
	void send(T&& msg, Node* dst) {
		send(&msg, dst);
	}
	
	template<typename T>
	void send_async(T&& msg, Node* dst) {
		RingBuffer::entry_t* entry;
		T* cpy = buffer.create<T>(entry);
		*cpy = msg;
		cpy->user = entry;
		cpy->callback = async_cb;
		send(cpy, dst);
	}
	
	template<typename T, typename R>
	T request(R&& req, Node* dst) {
		send(&req, dst);
		return req.res;
	}
	
	void send(Message* msg, Node* dst) {
		send_impl(msg, dst, false);
	}
	
	void send_async(Message* msg, Node* dst) {
		send_impl(msg, dst, true);
	}
	
	bool poll(Stream* stream, int64_t micro);
	
	void flush();
	
	void lock() {
		mutex.lock();
	}
	
	void unlock() {
		mutex.unlock();
	}
	
	virtual void notify() {
		cond.notify_all();
	}
	
	virtual void wait(int64_t micro) {
		cond.wait_for(ulock, std::chrono::microseconds(micro));
	}
	
	Message* collect(int64_t micro);
	
	virtual void fork(Object* object) = 0;
	
private:
	void exec(Object* object);
	
	void send_impl(Message* msg, Node* dst, bool async);
	
	void async_ack(Message* msg) {
		buffer.destroy<Message>((RingBuffer::entry_t*)msg->user);
	}
	
protected:
	Region memory;
	
private:
	vnl::util::spinlock sync;
	std::mutex mutex;
	std::condition_variable cond;
	std::unique_lock<std::mutex> ulock;
	std::mt19937_64 generator;
	
	Fiber* current = 0;
	std::atomic<int> waiting;
	Queue<Message*> queue;
	
	RingBuffer buffer;
	std::function<void(Message*)> async_cb;
	
	friend class Stream;
	friend class Object;
	friend class Fiber;
	friend class Page;
	
};



}}

#endif /* INCLUDE_PHY_ENGINE_H_ */
