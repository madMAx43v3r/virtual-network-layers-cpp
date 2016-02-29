/*
 * FiberEngine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include <chrono>
#include <boost/bind.hpp>
#include <boost/coroutine/symmetric_coroutine.hpp>

#include "phy/FiberEngine.h"

namespace vnl { namespace phy {

class FiberEngine::Fiber {
public:
	typedef boost::coroutines::symmetric_coroutine<void> fiber;
	
	std::function<void()> task;
	uint32_t tid = 0;
	
	Fiber(FiberEngine* engine)
		: 	engine(engine), _call(boost::bind(&Fiber::entry, this, boost::arg<1>())), task(0) {}
	
	void run() {
		while(true) {
			tid = 0;
			yield();
			task();
			engine->enqueue(this);
		}
	}
	
	void launch(const std::function<void()>& task_, uint32_t tid_) {
		task = task_;
		tid = tid_;
		call();
	}
	
	void start() {
		call();
	}
	
	bool poll(int millis) {
		poll();
		return result;
	}
	
	void notify(bool res) {
		if(polling) {
			result = res;
			call();
		}
	}
	
	void sent(Message* msg) {
		pending++;
		if(!msg->async && pending > 0) {
			wait();
		}
	}
	
	void acked() {
		pending--;
		if(pending == 0 && waiting) {
			call();
		}
	}
	
	void flush() {
		while(pending) {
			wait();
		}
	}
	
	int64_t timeout = 0;
	
protected:
	void poll() {
		polling = true;
		yield();
		polling = false;
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
	
	FiberEngine* engine;
	fiber::call_type _call;
	fiber::yield_type* _yield = 0;
	int pending = 0;
	bool result = false;
	bool polling = false;
	bool waiting = false;
	
};

FiberEngine::FiberEngine() {
	
}

FiberEngine::~FiberEngine() {
	for(auto it : avail) {
		delete it;
	}
}

void FiberEngine::sent(Message* msg) {
	if(current) {
		msg->impl = current;
		current->sent(msg);
	}
}

void FiberEngine::flush() {
	if(current) {
		current->flush();
	}
}

void FiberEngine::process(Stream* stream) {
	auto& queue = stream->impl;
	if(queue.size()) {
		auto iter = queue.begin();
		Fiber* fiber = (Fiber*)(*iter);
		queue.erase(iter);
		fiber->notify(true);
	}
}

void FiberEngine::ack(Message* msg) {
	Fiber* fiber = (Fiber*)msg->impl;
	fiber->acked();
}

bool FiberEngine::poll(Stream* stream, int millis) {
	if(current) {
		polling.insert(current);
		stream->impl.push_back(current);
		current->timeout = System::currentTimeMillis() + millis;
		bool res = current->poll(millis);
		polling.erase(current);
		return res;
	}
	return false;
}

taskid_t FiberEngine::launch(const std::function<void()>& func) {
	Fiber* fiber;
	if(avail.empty()) {
		fiber = new Fiber(this);
		fiber->start();
	} else {
		fiber = avail.back();
		avail.pop_back();
	}
	taskid_t task;
	task.id = nextid++;
	task.impl = fiber;
	fiber->launch(func, task.id);
	return task;
}

void FiberEngine::cancel(taskid_t task) {
	if(task.impl) {
		Fiber* fiber = (Fiber*)task.impl;
		if(fiber->tid == task.id) {
			delete fiber;
		}
	}
}

int FiberEngine::timeout() {
	int64_t now = System::currentTimeMillis();
	std::vector<Fiber*> list;
	int millis = 1000;
	while(true) {
		auto iter = polling.begin();
		while(iter != polling.end()) {
			Fiber* fiber = *iter;
			int diff = fiber->timeout - now;
			if(diff <= 0) {
				list.push_back(fiber);
				iter = polling.erase(iter);
				continue;
			} else if(diff < millis) {
				millis = diff;
			}
			iter++;
		}
		if(list.size()) {
			for(Fiber* fiber : list) {
				fiber->notify(false);
			}
			list.clear();
		} else {
			break;
		}
	}
	return millis;
}

void FiberEngine::enqueue(Fiber* fiber) {
	avail.push_back(fiber);
}





}}
