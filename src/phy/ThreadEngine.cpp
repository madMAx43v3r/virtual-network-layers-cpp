/*
 * ThreadEngine.cpp
 *
 *  Created on: Mar 3, 2016
 *      Author: mwittal
 */

#include <thread>

#include "phy/ThreadEngine.h"

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
		std::unique_lock<std::mutex> lock(engine->sync, std::adopt_lock);
		thread = new std::thread(&ThreadFiber::run, this);
		cond.wait(lock);
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
		engine->current = this;
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
		ulock.lock();
		engine->current = this;
		notify();
		try {
			while(true) {
				tid = 0;
				wait();
				task();
				engine->finished(this);
			}
		} catch(cancel_t&) {}
		engine->current = 0;
		ulock.unlock();
	}
	
	void wait() {
		engine->current = 0;
		waiting = true;
		cond.wait(ulock);
		waiting = false;
		if(docancel) {
			throw cancel_t();
		}
		engine->current = this;
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
	if(core_id >= 0) {
		Util::stick_to_core(core_id);
	}
	sync.lock();
	run();
	sync.unlock();
	dorun = true;
	notify();
	std::vector<Message*> inbox;
	while(dorun) {
		inbox.clear();
		while(dorun) {
			lock();
			if(acks.empty() && queue.empty()) {
				wait(1000);
			} else {
				Message* msg;
				while(acks.pop(msg)) {
					inbox.push_back(msg);
				}
				while(queue.pop(msg)) {
					inbox.push_back(msg);
				}
				unlock();
				break;
			}
			unlock();
		}
		sync.lock();
		for(Message* msg : inbox) {
			if(msg->isack) {
				msg->impl->acked(msg);
			} else {
				Stream* stream = msg->dst->get_stream(msg->sid);
				if(stream) {
					stream->push(msg);
					if(stream->sid == 0) {
						stream->obj->process();
					}
					Fiber* fiber;
					if(stream->impl.pop(fiber)) {
						fiber->notify(true);
					}
				}
			}
		}
		sync.unlock();
	}
	sync.lock();
	for(Fiber* fiber : fibers) {
		fiber->stop();
	}
	sync.unlock();
}

Fiber* ThreadEngine::create() {
	return new ThreadFiber(this);
}


}}
