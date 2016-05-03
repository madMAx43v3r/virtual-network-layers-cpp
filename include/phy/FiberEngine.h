/*
 * FiberEngine.h
 *
 *  Created on: Feb 28, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_FIBERENGINE_H_
#define INCLUDE_PHY_FIBERENGINE_H_

#include "phy/Fiber.h"
#include "phy/Engine.h"

namespace vnl { namespace phy {

class BoostFiber;

class FiberEngine : public Engine {
public:
	FiberEngine(int stack_size = 64*1024);
	
	~FiberEngine() {
		for(Fiber* fiber : fibers) {
			delete fiber;
		}
	}
	
	virtual void run(Object* object) override;
	
private:
	virtual void FiberEngine::fork(Object* object) override;
	
	int timeout();
	
private:
	int stack_size;
	
	List<BoostFiber*> fibers;
	Queue<BoostFiber*> avail;
	
};


}}

#endif /* INCLUDE_PHY_FIBERENGINE_H_ */
