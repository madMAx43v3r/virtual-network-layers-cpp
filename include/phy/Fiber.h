/*
 * Fiber.h
 *
 *  Created on: Feb 28, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_FIBER_H_
#define INCLUDE_PHY_FIBER_H_

#include <functional>

#include "phy/Engine.h"

namespace vnl { namespace phy {

class Fiber {
public:
	virtual ~Fiber() {};
	
	virtual void start() = 0;
	
	virtual void stop() = 0;
	
	virtual void launch(taskid_t task) = 0;
	
	virtual void sent(Message* msg) = 0;
	
	virtual void acked(Message* msg) = 0;
	
	virtual bool poll(int millis) = 0;
	
	virtual void notify(bool res) = 0;
	
	virtual void flush() = 0;
	
protected:
	taskid_t task;
	
	Fiber* get_current(Engine* engine) {
		return engine->current;
	}
	
	void set_current(Engine* engine, Fiber* fiber) {
		engine->current = fiber;
	}
	
	void finished(Engine* engine, Fiber* fiber);
	
private:
	std::vector<Stream*> waitlist;
	
	friend class Engine;
	
};


}}

#endif /* INCLUDE_PHY_FIBER_H_ */
