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

#include "util/mpsc_queue.h"
#include "phy/Message.h"
#include "phy/Node.h"
#include "phy/RingBuffer.h"
#include "System.h"
#include "Util.h"


namespace vnl { namespace phy {

class Page;
class Fiber;
class Stream;
class Object;

struct taskid_t {
	uint64_t id = 0;
	Fiber* impl = 0;
	std::function<void()> func;
};


class Engine : public vnl::phy::Node {
public:
	Engine();
	virtual ~Engine();
	
	static thread_local Engine* local;
	
	void start();
	void stop();
	
	void exec(const std::function<void()>& func) {
		SyncNode node;
		exec_t msg;
		msg.src = &node;
		msg.data = func;
		node.send(&msg, this);
	}
	
	virtual void receive(Message* msg) override {
		queue.push(msg);
		if(waiting-- == 1) {
			lock();
			notify();
			unlock();
		}
	}
	
	Page* get_page();
	void free_page(Page* page);
	
	typedef Generic<uint64_t, 0x5a8a106d> finished_t;
	typedef Generic<std::function<void()>, 0xde2a20fe> exec_t;
	
protected:
	bool dorun = true;
	
	std::vector<Fiber*> fibers;
	std::vector<Fiber*> avail;
	
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
	
	bool poll(Stream* stream, int64_t millis);
	
	void flush();
	
	taskid_t launch(const std::function<void()>& func);
	
	bool listen_on(const taskid_t& task, Node* dst);
	
	void lock() {
		mutex.lock();
	}
	
	void unlock() {
		mutex.unlock();
	}
	
	virtual void notify() {
		cond.notify_all();
	}
	
	virtual void wait(int millis) {
		cond.wait_for(ulock, std::chrono::milliseconds(millis));
	}
	
	virtual void mainloop() = 0;
	
	virtual Fiber* create() = 0;
	
	int collect(std::vector<Message*>& inbox, int timeout);
	
private:
	void send_impl(Message* msg, Node* dst, bool async);
	
	void entry() {
		mainloop();
	}
	
	void finished(Fiber* fiber) {
		avail.push_back(fiber);
	}
	
	void async_ack(Message* msg) {
		buffer.free((RingBuffer::entry_t*)msg->user);
	}
	
private:
	std::mutex mutex;
	std::condition_variable cond;
	std::unique_lock<std::mutex> ulock;
	std::mt19937_64 generator;
	
	Fiber* current = 0;
	std::atomic<int> waiting;
	vnl::util::mpsc_queue<Message*> queue;
	
	std::thread* thread;
	uint64_t nextid = 1;
	
	std::vector<Page*> pages;
	
	Region mem;
	RingBuffer buffer;
	std::function<void(Message*)> async_cb;
	
	friend class Message;
	friend class Stream;
	friend class Object;
	friend class Fiber;
	friend class Page;
	friend class Layer;
	
};



}}

#endif /* INCLUDE_PHY_ENGINE_H_ */
