/*
 * FiberEngine.cpp
 *
 *  Created on: Feb 28, 2016
 *      Author: mad
 */

#include <chrono>
#include <boost/bind.hpp>
#include <boost/coroutine/symmetric_coroutine.hpp>

#include "phy/FiberEngine.h"
#include "phy/Stream.h"

namespace vnl { namespace phy {

class BoostFiber : public Fiber {
public:
	typedef boost::coroutines::symmetric_coroutine<void> fiber;
	
	BoostFiber(FiberEngine* engine)
		: 	engine(engine), _call(boost::bind(&BoostFiber::entry, this, boost::arg<1>())) {}
	
	virtual void launch(taskid_t task_) override {
		task = task_;
		call();
	}
	
	virtual void start() override {
		call();
	}
	
	virtual void stop() override {
		
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
			call();
		}
	}
	
	virtual bool poll(int millis) override {
		timeout = System::currentTimeMillis() + millis;
		polling = true;
		yield();
		polling = false;
		return result;
	}
	
	virtual void notify(bool res) override {
		if(polling) {
			result = res;
			call();
		}
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
		while(true) {
			yield();
			task.func();
			finished(engine, this);
		}
	}
	
	void wait() {
		waiting = true;
		yield();
		waiting = false;
	}
	
	void call() {
		Fiber* tmp = get_current(engine);
		set_current(engine, this);
		_call();
		set_current(engine, tmp);
	}
	
	void yield() {
		(*_yield)();
		if(cbs.size()) {
			for(Message* msg : cbs) {
				msg->callback(msg);
			}
			cbs.clear();
		}
	}
	
	void entry(fiber::yield_type& yield_) {
		_yield = &yield_;
		run();
	}
	
	FiberEngine* engine;
	std::vector<Message*> cbs;
	fiber::call_type _call;
	fiber::yield_type* _yield = 0;
	int pending = 0;
	bool result = false;
	bool waiting = false;
	
};


void FiberEngine::mainloop() {
	local = this;
	std::vector<Stream*> list;
	std::vector<Message*> inbox;
	while(dorun) {
		inbox.clear();
		while(dorun) {
			int to = timeout();
			if(collect(inbox, to)) {
				break;
			}
		}
		list.clear();
		for(Message* msg : inbox) {
			if(msg->mid == Engine::exec_t::id) {
				launch(((Engine::exec_t*)msg)->data, 0);
			} else if(msg->isack) {
				msg->impl->acked(msg);
			} else {
				msg->dst->push(msg);
				list.push_back(msg->dst);
			}
		}
		for(Stream* stream : list) {
			if(!stream->queue.empty()) {
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
}

Fiber* FiberEngine::create() {
	return new BoostFiber(this);
}

int FiberEngine::timeout() {
	int64_t now = System::currentTimeMillis();
	std::vector<BoostFiber*> list;
	int millis = 1000;
	while(true) {
		list.clear();
		for(Fiber* ptr : fibers) {
			BoostFiber* fiber = (BoostFiber*)ptr;
			if(fiber->polling) {
				int diff = fiber->timeout - now;
				if(diff <= 0) {
					list.push_back(fiber);
				} else if(diff < millis) {
					millis = diff;
				}
			}
		}
		if(list.size()) {
			for(BoostFiber* fiber : list) {
				fiber->notify(false);
			}
		} else {
			break;
		}
	}
	return millis;
}



}}
