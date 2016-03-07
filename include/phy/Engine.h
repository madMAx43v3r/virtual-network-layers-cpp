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
	std::function<void()> func;
	taskid_t() : id(0), impl(0) {}
};


class Engine {
public:
	Engine();
	virtual ~Engine();
	
	static thread_local Engine* local;
	
	void start();
	void stop();
	
	void exec(const std::function<void()>& func) {
		exec_info_t info;
		info.func = func;
		exec_t msg;
		msg.data = std::bind(&Engine::do_exec, this, &info);
		receive(&msg);
		std::unique_lock<std::mutex> ulock(mutex);
		info.cond.wait(ulock);
	}
	
protected:
	bool dorun = true;
	
	std::vector<Fiber*> fibers;
	std::vector<Fiber*> avail;
	
	uint64_t rand() {
		return Util::hash64(generator());
	}
	
	void send(Message* msg, Stream* dst, bool async);
	
	bool poll(Stream* stream, int millis);
	
	void flush();
	
	taskid_t launch(const std::function<void()>& func, Stream* stream);
	
	void wait_on(const taskid_t& task, Stream* stream);
	
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
	
	typedef Generic<std::function<void()>, 0xde2a20fe> exec_t;
	typedef Generic<uint64_t, 0x5a8a106d> finished_t;
	
private:
	struct exec_info_t {
		std::function<void()> func;
		std::condition_variable cond;
	};
	
	void entry() {
		mainloop();
	}
	
	void do_exec(exec_info_t* info) {
		info->func();
		std::unique_lock<std::mutex> ulock(mutex);
		info->cond.notify_all();
	}
	
	void wait() {
		cond.wait(ulock);
	}
	
	void receive(Message* msg) {
		queue.push(msg);
		if(waiting-- == 1) {
			lock();
			notify();
			unlock();
		}
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
	
	Fiber* current = 0;
	std::atomic<int> waiting;
	vnl::util::mpsc_queue<Message*> queue;
	
	std::thread* thread;
	uint64_t nextid = 1;
	
	std::vector<Page*> pages;
	
	friend class Message;
	friend class Stream;
	friend class Object;
	friend class Fiber;
	friend class Page;
	
};



}}

#endif /* INCLUDE_PHY_ENGINE_H_ */
