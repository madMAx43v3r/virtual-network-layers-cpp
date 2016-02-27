/*
 * FiberEngine.h
 *
 *  Created on: Dec 17, 2015
 *      Author: mad
 */

#ifndef INCLUDE_PHY_FIBERENGINE_H_
#define INCLUDE_PHY_FIBERENGINE_H_

#include <vector>
#include <list>

#include "Engine.h"

namespace vnl { namespace phy {

class FiberEngine : public Engine {
public:
	FiberEngine();
	~FiberEngine();
	
	virtual void send(Message* msg) override;
	virtual void flush() override;
	virtual void handle(Message* msg, Stream* stream) override;
	virtual void ack(Message* msg) override;
	virtual bool poll(Stream* stream, int millis) override;
	virtual void* launch(Runnable* task) override;
	virtual void cancel(void* task) override;
	virtual int timeout() override;
	
protected:
	class Fiber;
	
	void enqueue(Fiber* fiber);
	
private:
	Fiber* current = 0;
	std::vector<Fiber*> avail;
	std::list<Fiber*> polling;
	
};

}}

#endif /* INCLUDE_PHY_FIBERENGINE_H_ */
