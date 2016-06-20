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
	
	virtual void fork(Module* object) override;
	
	void run();
	
	static void spawn(Module* object, int stack_size = default_stack) {
		std::thread thread(std::bind(&FiberEngine::entry, object, stack_size));
		thread.detach();
		Registry::ping(object->getMAC());
	}
	
protected:
	virtual void send_impl(Message* msg, Basic* dst, bool async) override;
	
	virtual bool poll(Stream* stream, int64_t micros) override;
	
	virtual void flush() override;
	
private:
	int timeout();
	
	static void entry(Module* object, int stack_size) {
		FiberEngine engine(stack_size);
		engine.fork(object);
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
