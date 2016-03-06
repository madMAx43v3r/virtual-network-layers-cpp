/*
 * ThreadEngine.cpp
 *
 *  Created on: Mar 3, 2016
 *      Author: mwittal
 */

#include <thread>
#include <string.h>

#include "phy/ThreadEngine.h"
#include "phy/Stream.h"

namespace vnl { namespace phy {

class ThreadFiber : public Fiber {
public:
	class cancel_t {};
	
	ThreadFiber(ThreadEngine* engine)
		: 	engine(engine), ulock(engine->sync, std::defer_lock), thread(0)
	{
	}
	
	~ThreadFiber() {
		thread->join();
		delete thread;
	}
	
	virtual void launch(const std::function<void()>& task_, uint32_t tid_) override {
		task = task_;
		tid = tid_;
		notify();
	}
	
	virtual void start() override {
		thread = new std::thread(&ThreadFiber::run, this);
	}
	
	virtual void stop() override {
		docancel = true;
		notify();
	}
	
	virtual void sent(Message* msg) override {
		pending++;
		if(!msg->async && pending > 0) {
			wait();
		}
	}
	
	virtual void acked(Message* msg) override {
		if(msg->callback) {
			cbs.push_back(msg);
		}
		pending--;
		if(pending == 0 && waiting) {
			notify();
		}
	}
	
	virtual bool poll(int millis) override {
		bool res = cond.wait_for(ulock, std::chrono::milliseconds(millis)) == std::cv_status::no_timeout;
		if(docancel) {
			throw cancel_t();
		}
		set_current(engine, this);
		check_cbs();
		return res;
	}
	
	virtual void notify(bool res) override {
		notify();
	}
	
	virtual void flush() override {
		while(pending) {
			wait();
		}
	}
	
	int64_t timeout = 0;
	bool polling = false;
	
protected:
	void run() {
		Engine::local = engine;
		ulock.lock();
		set_current(engine, this);
		try {
			while(true) {
				task();
				tid = 0;
				finished(engine, this);
				wait();
			}
		} catch(cancel_t&) {}
		set_current(engine, 0);
		ulock.unlock();
	}
	
	void wait() {
		set_current(engine, 0);
		waiting = true;
		cond.wait(ulock);
		waiting = false;
		if(docancel) {
			throw cancel_t();
		}
		set_current(engine, this);
		check_cbs();
	}
	
	void notify() {
		cond.notify_all();
	}
	
	void check_cbs() {
		if(cbs.size()) {
			for(Message* msg : cbs) {
				msg->callback(msg);
			}
			cbs.clear();
		}
	}
	
	ThreadEngine* engine;
	std::function<void()> task;
	std::vector<Message*> cbs;
	std::unique_lock<std::mutex> ulock;
	std::condition_variable cond;
	std::thread* thread;
	int pending = 0;
	bool waiting = false;
	bool docancel = false;
	
	
};


void ThreadEngine::mainloop() {
	local = this;
	sync.lock();
	run();
	lock();
	dorun = true;
	notify();
	unlock();
	std::vector<Message*> inbox;
	while(dorun) {
		sync.unlock();
		inbox.clear();
		while(dorun) {
			lock();
			int nack = acks.size();
			int nmsg = queue.size();
			int total = nack + nmsg;
			if(total) {
				inbox.resize(total);
				if(nack) { memcpy(&inbox[0], 	&acks[0], 	nack * sizeof(void*)); acks.clear(); }
				if(nmsg) { memcpy(&inbox[nack], &queue[0], 	nmsg * sizeof(void*)); queue.clear(); }
				unlock();
				break;
			} else {
				wait(1000);
			}
			unlock();
		}
		sync.lock();
		for(Message* msg : inbox) {
			if(msg->isack) {
				msg->impl->acked(msg);
			} else {
				Stream* stream = msg->dst;
				stream->push(msg);
				Fiber* fiber;
				if(stream->impl.pop(fiber)) {
					fiber->notify(true);
				}
			}
		}
	}
	for(Fiber* fiber : fibers) {
		fiber->stop();
	}
	sync.unlock();
}

Fiber* ThreadEngine::create() {
	return new ThreadFiber(this);
}


}}
