/*
 * FiberEngine.h
 *
 *  Created on: Dec 17, 2015
 *      Author: mad
 */

#ifndef INCLUDE_PHY_FIBERENGINE_H_
#define INCLUDE_PHY_FIBERENGINE_H_

#include <vector>
#include <set>

#include "Engine.h"

namespace vnl { namespace phy {

class FiberEngine : public Engine {
public:
	FiberEngine();
	~FiberEngine();
	
	virtual void sent(Message* msg) override;
	virtual void ack(Message* msg) override;
	virtual void flush() override;
	virtual void process(Stream* stream) override;
	virtual bool poll(Stream* stream, int millis) override;
	virtual taskid_t launch(const std::function<void()>& func) override;
	virtual void cancel(taskid_t task) override;
	virtual int timeout() override;
	
protected:
	class Fiber;
	
	void enqueue(Fiber* fiber);
	
private:
	Fiber* current = 0;
	std::vector<Fiber*> avail;
	std::set<Fiber*> polling;
	
	uint32_t nextid = 1;
	
};

}}

#endif /* INCLUDE_PHY_FIBERENGINE_H_ */
