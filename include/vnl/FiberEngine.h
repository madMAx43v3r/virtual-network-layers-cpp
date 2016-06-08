/*
 * FiberEngine.h
 *
 *  Created on: Feb 28, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_FIBERENGINE_H_
#define INCLUDE_PHY_FIBERENGINE_H_

#include <thread>

#include "vnl/Engine.h"
#include "vnl/Array.h"


namespace vnl {

class Fiber;

class FiberEngine : public Engine {
public:
	FiberEngine(int stack_size = 64*1024);
	
	virtual ~FiberEngine();
	
	virtual void fork(Module* object) override;
	
	virtual void exec(Module* object) override;
	
	void detach(Module* object) {
		assert(is_detached == false);
		std::thread thread(std::bind(&FiberEngine::entry, this, object));
		thread.detach();
		is_detached = true;
	}
	
protected:
	virtual void send_impl(Base* src, Message* msg, Base* dst, bool async) override;
	
	virtual bool poll(Stream* stream, int64_t micros) override;
	
	virtual void flush() override;
	
private:
	int timeout();
	
	void entry(Module* object) {
		exec(object);
	}
	
private:
	int stack_size;
	bool is_detached = false;
	
	Fiber* current = 0;
	
	Array<Fiber*> fibers;
	Queue<Fiber*> avail;
	
	friend class Fiber;
	
};


}

#endif /* INCLUDE_PHY_FIBERENGINE_H_ */
