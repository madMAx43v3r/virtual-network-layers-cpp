/*
 * FiberEngine.cpp
 *
 *  Created on: Feb 28, 2016
 *      Author: mad
 */

#include "vnl/build/config.h"

#ifdef VNL_HAVE_BOOST_COROUTINE

#include <chrono>
#include <boost/bind.hpp>
#include <boost/coroutine/symmetric_coroutine.hpp>
#include <boost/coroutine/protected_stack_allocator.hpp>

#include "vnl/phy/FiberEngine.h"
#include "vnl/phy/Stream.h"


namespace vnl { namespace phy {

class Fiber {
public:
	typedef boost::coroutines::symmetric_coroutine<void> fiber;
	
	Fiber(FiberEngine* engine, int stack_size)
		: 	engine(engine),
			_call(	boost::bind(&Fiber::entry, this, boost::arg<1>()),
					boost::coroutines::attributes(stack_size),
					boost::coroutines::protected_stack_allocator()	)
	{
		call();
	}
	
	void exec(Object* obj_) {
		obj = obj_;
		call();
	}
	
	void sent(Message* msg, bool async) {
		pending++;
		if(!async && pending > 0) {
			wait_msg = msg;
			while(wait_msg) {
				wait();
			}
		}
	}
	
	void acked(Message* msg) {
		pending--;
		if(msg == wait_msg) {
			wait_msg = 0;
		}
		if(msg->callback) {
			engine->current = this;
			(*msg->callback)(msg);
			engine->current = 0;
		}
		msg->release();
		if(waiting) {
			call();
		}
	}
	
	bool poll(Stream* stream, int64_t micro) {
		stream->_impl = this;
		if(micro >= 0) {
			deadline = System::currentTimeMicros() + micro;
		} else {
			deadline = 0;
		}
		polling = stream;
		yield();
		polling = 0;
		stream->_impl = 0;
		return result;
	}
	
	void notify(bool res) {
		if(polling) {
			result = res;
			call();
		}
	}
	
	void flush() {
		while(pending > 0) {
			wait();
		}
	}
	
	int64_t deadline = 0;
	Stream* polling = 0;
	
protected:
	void run() {
		while(true) {
			yield();
			engine->Engine::exec(obj);
			engine->avail.push(this);
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
	
	FiberEngine* engine;
	fiber::call_type _call;
	fiber::yield_type* _yield = 0;
	Object* obj;
	int pending = 0;
	bool result = false;
	bool waiting = false;
	Message* wait_msg = 0;
	
};


FiberEngine::FiberEngine(int stack_size)
	:	stack_size(stack_size), avail(memory)
{
}

FiberEngine::~FiberEngine() {
	for(Fiber* fiber : fibers) {
		delete fiber;
	}
}

void FiberEngine::exec(Object* object) {
	assert(Engine::local == this);
	fork(object);
	while(dorun) {
		int64_t micros = timeout();
		while(true) {
			Message* msg = collect(micros);
			if(msg) {
				if(msg->isack) {
					assert(msg->_impl);
					((Fiber*)msg->_impl)->acked(msg);
				} else {
					msg->dst->receive(msg);
					Fiber* fiber = (Fiber*)msg->dst->_impl;
					if(fiber) {
						fiber->notify(true);
					}
				}
			} else {
				break;
			}
		}
	}
}

void FiberEngine::send_impl(Node* src, Message* msg, Node* dst, bool async) {
	assert(msg->isack == false);
	assert(dst);
	assert(current);
	
	msg->src = src;
	msg->_impl = current;
	dst->receive(msg);
	current->sent(msg, async);
}

bool FiberEngine::poll(Stream* stream, int64_t micros) {
	assert(stream);
	assert(stream->_impl == 0);
	assert(stream->getEngine() == this);
	assert(current);
	
	return current->poll(stream, micros);
}

void FiberEngine::flush() {
	assert(Engine::local == this);
	assert(current);
	
	current->flush();
}

void FiberEngine::fork(Object* object) {
	assert(Engine::local == this);
	Fiber* fiber;
	if(!avail.pop(fiber)) {
		fiber = new Fiber(this, stack_size);
		fibers.push_back(fiber);
	}
	fiber->exec(object);
}

int FiberEngine::timeout() {
	Region mem;
	int64_t now = System::currentTimeMicros();
	int64_t micros = 1000*1000;
	Queue<Fiber*> list(mem);
	while(true) {
		for(Fiber* fiber : fibers) {
			if(fiber->polling && fiber->deadline > 0) {
				int64_t diff = fiber->deadline - now;
				if(diff <= 0) {
					list.push(fiber);
				} else if(diff < micros) {
					micros = diff;
				}
			}
		}
		if(!list.empty()) {
			Fiber* fiber;
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

#endif // VNL_HAVE_BOOST_COROUTINE
