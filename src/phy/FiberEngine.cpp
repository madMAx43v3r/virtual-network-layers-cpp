/*
 * FiberEngine.cpp
 *
 *  Created on: Feb 28, 2016
 *      Author: mad
 */

#include <chrono>
#include <boost/bind.hpp>
#include <boost/coroutine/symmetric_coroutine.hpp>
#include <boost/coroutine/protected_stack_allocator.hpp>

#include "phy/FiberEngine.h"
#include "phy/Stream.h"

namespace vnl { namespace phy {

class BoostFiber : public Fiber {
public:
	typedef boost::coroutines::symmetric_coroutine<void> fiber;
	
	BoostFiber(FiberEngine* engine, int stack_size)
		: 	engine(engine),
			_call(	boost::bind(&BoostFiber::entry, this, boost::arg<1>()),
					boost::coroutines::attributes(stack_size),
					boost::coroutines::protected_stack_allocator()	)
	{
		call();
	}
	
	void exec(Object* obj_) {
		obj = obj_;
		call();
	}
	
	virtual void sent(Message* msg, bool async) override {
		pending++;
		if(!async && pending > 0) {
			wait_msg = msg;
			wait();
			wait_msg = 0;
		}
	}
	
	void acked(Message* msg) {
		if(msg->callback) {
			cbs.push_back(msg);
		}
		pending--;
		if(waiting && (wait_msg == msg || pending == 0)) {
			call();
		}
	}
	
	virtual bool poll(int64_t micro) override {
		timeout = System::currentTimeMicros() + micro;
		polling = true;
		yield();
		polling = false;
		return result;
	}
	
	void notify(bool res) {
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
			do_exec(engine, obj);
			engine->avail.push(this);
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
	Object* obj;
	int pending = 0;
	bool result = false;
	bool waiting = false;
	Message* wait_msg = 0;
	
};


FiberEngine::FiberEngine(int stack_size)
	:	stack_size(stack_size), fibers(memory), avail(memory)
{
}

void FiberEngine::run(Object* object) {
	assert(Engine::local == this);
	Region mem;
	fork(object);
	Queue<Node*> list(&mem);
	while(dorun) {
		int64_t micros = timeout();
		while(true) {
			Message* msg = collect(micros);
			if(msg) {
				if(msg->isack) {
					((BoostFiber*)msg->impl)->acked(msg);
				} else {
					Node* node = msg->dst;
					node->receive(msg);
					list.push(node);
				}
			} else {
				break;
			}
		}
		Node* node;
		while(list.pop(node)) {
			if(node->impl) {
				((BoostFiber*)node->impl)->notify(true);
			}
		}
	}
}

void FiberEngine::fork(Object* object) {
	assert(Engine::local == this);
	BoostFiber* fiber;
	if(!avail.pop(fiber)) {
		fiber = new BoostFiber(this);
		fibers.push(fiber);
	}
	fiber->exec(object);
}

int FiberEngine::timeout() {
	Region mem;
	int64_t now = System::currentTimeMicros();
	int64_t micros = 9223372036854775808LL;
	Queue<BoostFiber*> list(&mem);
	while(true) {
		for(BoostFiber* fiber : fibers) {
			if(fiber->polling) {
				int64_t diff = fiber->timeout - now;
				if(diff <= 0) {
					list.push(fiber);
				} else if(diff < micros) {
					micros = diff;
				}
			}
		}
		if(!list.empty()) {
			BoostFiber* fiber;
			while(list.pop(fiber)) {
				fiber->notify(false);
			}
		} else {
			break;
		}
	}
	return micros;
}



}}
