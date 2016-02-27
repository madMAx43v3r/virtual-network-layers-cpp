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
	
	Runnable* task;
	
	Fiber(FiberEngine* engine)
		: 	engine(engine), _call(boost::bind(&Fiber::entry, this, boost::arg<1>())), task(0) {}
	
	void* launch(Runnable* task_) {
		task = task_;
		call();
		if(task) { return this; }
		return 0;
	}
	
	void run() {
		while(true) {
			yield();
			task->run();
			task = 0;
			engine->enqueue(this);
		}
	}
	
	void start() {
		call();
	}
	
	bool poll(int millis) {
		timeout = System::currentTimeMillis() + millis;
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
	bool result = false;
	
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

void FiberEngine::send(Message* msg) {
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

void FiberEngine::handle(Message* msg, Stream* stream) {
	auto& queue = stream->impl;
	if(queue.size()) {
		auto fiber = queue.begin();
		((Fiber*)(*fiber))->notify(true);
		queue.erase(fiber);
	}
}

void FiberEngine::ack(Message* msg) {
	Fiber* fiber = (Fiber*)msg->impl;
	fiber->acked();
}

bool FiberEngine::poll(Stream* stream, int millis) {
	if(current) {
		polling.push_back(current);
		stream->impl.push_back(current);
		return current->poll(millis);
	}
	return false;
}

void* FiberEngine::launch(Runnable* task) {
	Fiber* fiber;
	if(avail.empty()) {
		fiber = new Fiber(this);
		fiber->start();
	} else {
		fiber = avail.back();
		avail.pop_back();
	}
	return fiber->launch(task);
}

void FiberEngine::cancel(void* task) {
	Fiber* fiber = (Fiber*)task;
	delete fiber;
}

int FiberEngine::timeout() {
	int64_t now = System::currentTimeMillis();
	std::vector<Fiber*> list;
	int millis = 1000;
	while(true) {
		for(auto iter = polling.begin(); iter != polling.end(); ++iter) {
			Fiber* fiber = *iter;
			int diff = fiber->timeout - now;
			if(diff <= 0) {
				list.push_back(fiber);
				polling.erase(iter);
			} else if(diff < millis) {
				millis = diff;
			}
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
