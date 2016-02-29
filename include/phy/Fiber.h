/*
 * Fiber.h
 *
 *  Created on: Feb 28, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_FIBER_H_
#define INCLUDE_PHY_FIBER_H_

#include <functional>

#include "phy/Message.h"

namespace vnl { namespace phy {

class Engine;

class Fiber {
public:
	Fiber(Engine* engine) : engine(engine) {}
	
	virtual ~Fiber() {};
	
	virtual void start() = 0;
	
	virtual void stop() = 0;
	
	virtual void launch(const std::function<void()>& task, uint32_t tid) = 0;
	
	virtual void sent(Message* msg) = 0;
	
	virtual void acked() = 0;
	
	virtual bool poll(int millis) = 0;
	
	virtual void notify(bool res) = 0;
	
	virtual void flush() = 0;
	
	void finished();
	
	Engine* engine;
	uint32_t tid = 0;
	
};


}}

#endif /* INCLUDE_PHY_FIBER_H_ */
