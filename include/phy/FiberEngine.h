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

class FiberEngine : public Engine {
protected:
	FiberEngine(int stack_size = 64*1024);
	
	virtual void mainloop() override;
	
	virtual Fiber* create() override;
	
private:
	int timeout();
	
private:
	int stack_size;
	
};


}}

#endif /* INCLUDE_PHY_FIBERENGINE_H_ */
