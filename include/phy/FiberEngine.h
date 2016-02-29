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
	
	virtual Fiber* create() override;
	virtual int timeout() override;
	
};


}}

#endif /* INCLUDE_PHY_FIBERENGINE_H_ */
