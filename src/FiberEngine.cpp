/*
 * FiberEngine.cpp
 *
 *  Created on: Feb 28, 2016
 *      Author: mad
 */

#include "vnl/build/config.h"
#include <vnl/Object.h>

#ifdef VNL_HAVE_FIBER_ENGINE

#include <chrono>
#include <boost/bind.hpp>
#include <boost/coroutine/symmetric_coroutine.hpp>
#include <boost/coroutine/protected_stack_allocator.hpp>

#include <vnl/FiberEngine.h>


namespace vnl {

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
	
	void exec(Module* obj_) {
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
			deadline = currentTimeMicros() + micro;
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
			Message msg;
			engine->exec(obj, &msg);
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
	Module* obj;
	int pending = 0;
	bool result = false;
	bool waiting = false;
	Message* wait_msg = 0;
	
};


FiberEngine::FiberEngine(int stack_size)
	:	stack_size(stack_size)
{
}

FiberEngine::~FiberEngine() {
	for(Fiber* fiber : fibers) {
		delete fiber;
	}
}

void FiberEngine::run() {
	vnl::Queue<Message*> inbox;
	while(fibers.size() > avail.size()) {
		int64_t micros = timeout();
		collect(micros, inbox);
		Message* msg = 0;
		while(inbox.pop(msg)) {
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
		}
	}
}

void FiberEngine::send_impl(Message* msg, Basic* dst, bool async) {
	assert(msg->isack == false);
	assert(current);
	
	msg->src = this;
	msg->_impl = current;
	dst->receive(msg);
	current->sent(msg, async);
}

bool FiberEngine::poll(Stream* stream, int64_t micros) {
	assert(stream->_impl == 0);
	assert(stream->getEngine() == this);
	assert(current);
	
	return current->poll(stream, micros);
}

void FiberEngine::flush() {
	assert(current);
	current->flush();
}

void FiberEngine::fork(Module* object) {
	Fiber* fiber;
	if(!avail.pop(fiber)) {
		fiber = new Fiber(this, stack_size);
		fibers.push_back(fiber);
	}
	Message msg;
	fiber->exec(object, &msg);
}

int FiberEngine::timeout() {
	int64_t now = currentTimeMicros();
	int64_t micros = 1000*1000;
	Queue<Fiber*> list;
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


class FiberServer : public Module {
public:
	static FiberServer* instance;
	
	typedef MessageType<Module*, 0xede39599> fork_t;
	
protected:
	virtual void main(Engine* engine_) {
		engine = engine_;
		run();
	}
	
	virtual bool handle(Message* msg) {
		if(msg->msg_id == fork_t::MID) {
			engine->fork(((fork_t*)msg)->data);
		}
		return Node::handle(msg);
	}
	
private:
	Engine* engine;
	
};

FiberServer* FiberServer::instance = 0;


Address fork(Module* object) {
	if(!FiberServer::instance) {
		FiberServer::instance = new FiberServer();
		vnl::spawn(FiberServer::instance);
	}
	Address addr = object->my_address;
	Actor actor;
	FiberServer::fork_t fork(object);
	actor.send(&fork, FiberServer::instance);
	return addr;
}



} // vnl

#else // VNL_HAVE_FIBER_ENGINE

#include "vnl/Engine.h"

namespace vnl {

Address fork(Object* object) {
	Address addr = object->my_address;
	vnl::spawn(object);
	return addr;
}

} // vnl

#endif // VNL_HAVE_FIBER_ENGINE
