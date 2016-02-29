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

namespace vnl { namespace phy {

class BoostFiber : public Fiber {
public:
	typedef boost::coroutines::symmetric_coroutine<void> fiber;
	
	BoostFiber(Engine* engine)
		: 	Fiber::Fiber(engine), _call(boost::bind(&BoostFiber::entry, this, boost::arg<1>())) {}
	
	virtual void launch(const std::function<void()>& task_, uint32_t tid_) override {
		task = task_;
		tid = tid_;
		call();
	}
	
	virtual void start() override {
		call();
	}
	
	virtual void stop() override {
		// nothing to do
	}
	
	virtual void sent(Message* msg) override {
		pending++;
		if(!msg->async && pending > 0) {
			wait();
		}
	}
	
	virtual void acked() override {
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
			tid = 0;
			yield();
			task();
			finished();
		}
	}
	
	void wait() {
		waiting = true;
		yield();
		waiting = false;
	}
	
	void call() {
		Fiber* tmp = engine->current;
		engine->current = this;
		_call();
		engine->current = tmp;
	}
	
	void yield() {
		(*_yield)();
	}
	
	void entry(fiber::yield_type& yield_) {
		_yield = &yield_;
		run();
	}
	
	std::function<void()> task;
	fiber::call_type _call;
	fiber::yield_type* _yield = 0;
	int pending = 0;
	bool result = false;
	bool waiting = false;
	
	
};


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
