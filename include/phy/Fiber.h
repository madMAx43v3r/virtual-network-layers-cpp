/*
 * Fiber.h
 *
 *  Created on: Feb 28, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_FIBER_H_
#define INCLUDE_PHY_FIBER_H_

#include "phy/Engine.h"

namespace vnl { namespace phy {

class Object;

class Fiber {
public:
	virtual ~Fiber() {};
	
	virtual void sent(Message* msg, bool async) = 0;
	
	virtual bool poll(int64_t micro) = 0;
	
	virtual void flush() = 0;
	
protected:
	Fiber* get_current(Engine* engine) {
		return engine->current;
	}
	
	void set_current(Engine* engine, Fiber* fiber) {
		engine->current = fiber;
	}
	
	void do_exec(Engine* engine, Object* object) {
		engine->exec(object);
	}
	
};


}}

#endif /* INCLUDE_PHY_FIBER_H_ */
