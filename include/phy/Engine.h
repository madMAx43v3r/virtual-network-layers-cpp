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

#include "phy/Message.h"
#include "Runnable.h"
#include "System.h"
#include "Util.h"

namespace vnl { namespace phy {

class Page;
class Fiber;
class Stream;
class Object;

struct taskid_t {
	uint64_t id;
	Fiber* impl;
	taskid_t() : id(0), impl(0) {}
};


class Engine : public vnl::Runnable {
public:
	Engine();
	virtual ~Engine();
	
	static thread_local Engine* local;
	
	void start();
	void stop();
	
protected:
	bool dorun = false;
	
	std::vector<Message*> queue;
	std::vector<Message*> acks;
	
	std::unordered_set<Fiber*> fibers;
	std::vector<Fiber*> avail;
	
	uint64_t rand() {
		return Util::hash64(generator());
	}
	
	void send(Message* msg, Stream* dst, bool async);
	
	bool poll(Stream* stream, int millis);
	
	void flush();
	
	taskid_t launch(const std::function<void()>& func);
	
	void cancel(taskid_t task);
	
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
	
private:
	void entry() {
		mainloop();
	}
	
	void wait() {
		cond.wait(ulock);
	}
	
	void receive(Message* msg) {
		lock();
		if(msg->isack) {
			acks.push_back(msg);
		} else {
			queue.push_back(msg);
		}
		notify();
		unlock();
	}
	
	void finished(Fiber* fiber) {
		avail.push_back(fiber);
	}
	
	Page* get_page();
	
	void free_page(Page* page);
	
private:
	std::mutex mutex;
	std::condition_variable cond;
	std::unique_lock<std::mutex> ulock;
	std::default_random_engine generator;
	
	std::thread* thread;
	uint64_t nextid = 1;
	
	Fiber* current = 0;
	
	std::vector<Page*> pages;
	
	friend class Message;
	friend class Stream;
	friend class Object;
	friend class Fiber;
	friend class Page;
	
};



}}

#endif /* INCLUDE_PHY_ENGINE_H_ */
