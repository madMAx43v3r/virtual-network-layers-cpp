/*
 * FiberEngine.h
 *
 *  Created on: Feb 28, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_FIBERENGINE_H_
#define INCLUDE_PHY_FIBERENGINE_H_

#include <thread>

#include "vnl/build/config.h"
#include "vnl/Engine.h"
#include "vnl/Array.h"


namespace vnl {

class Fiber;

class FiberEngine : public Engine {
public:
	static const int default_stack = 64*1024;
	
	FiberEngine(int stack_size = default_stack);
	
	virtual ~FiberEngine();
	
	void run();
	
	static void spawn(Object* object, int stack_size = default_stack) {
		Actor sync;
		Message msg;
		msg.src = &sync;
		std::thread thread(&FiberEngine::entry, object, &msg, stack_size);
		thread.detach();
		sync.wait();
	}
	
protected:
	virtual void fork(Object* object);
	
	virtual void send_impl(Message* msg, Basic* dst, bool async);
	
	virtual bool poll(Stream* stream, int64_t micros) override;
	
	virtual void flush() override;
	
private:
	int timeout();
	
	static void entry(Object* object, Message* msg, int stack_size) {
		FiberEngine engine(stack_size);
		engine.fork(object);
		msg->ack();
		engine.run();
	}
	
private:
	int stack_size;
	
	Fiber* current = 0;
	
	Array<Fiber*> fibers;
	Queue<Fiber*> avail;
	
	friend class Fiber;
	
};



}

#endif /* INCLUDE_PHY_FIBERENGINE_H_ */
