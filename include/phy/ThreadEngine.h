/*
 * ThreadEngine.h
 *
 *  Created on: Mar 3, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_PHY_THREADENGINE_H_
#define INCLUDE_PHY_THREADENGINE_H_

#include <mutex>
#include <condition_variable>

#include "phy/Fiber.h"
#include "phy/Engine.h"

namespace vnl { namespace phy {

class ThreadFiber;

class ThreadEngine : public Engine {
protected:
	
	virtual void mainloop() override;
	
	virtual Fiber* create() override;
	
private:
	std::mutex sync;
	
	friend class ThreadFiber;
	
};


}}

#endif /* INCLUDE_PHY_THREADENGINE_H_ */
