/*
 * FiberEngine.h
 *
 *  Created on: Dec 17, 2015
 *      Author: mad
 */

#ifndef INCLUDE_PHY_FIBERENGINE_H_
#define INCLUDE_PHY_FIBERENGINE_H_

#include <vector>
#include <unordered_map>

#include "Engine.h"

namespace vnl { namespace phy {

class FiberEngine : public Engine {
public:
	FiberEngine(int N);
	~FiberEngine();
	
	virtual void send(Message* msg) override;
	virtual void flush() override;
	virtual void handle(Message* msg, Stream* stream) override;
	virtual void open(Stream* stream) override;
	virtual void close(Stream* stream) override;
	virtual bool poll(Stream* stream, int millis) override;
	virtual uint64_t launch(Runnable* task) override;
	virtual void cancel(uint64_t tid) override;
	virtual int timeout() override;
	
protected:
	class Fiber;
	class Worker;
	class Task;
	
	void enqueue(Task* task);
	
private:
	int N;
	Fiber* current = 0;
	Worker** workers;
	std::vector<Worker*> avail;
	std::vector<Task*> finished;
	std::unordered_map<uint64_t, std::vector<Fiber*> > polling;
	std::unordered_map<uint64_t, Task*> tasks;
	
};

}}

#endif /* INCLUDE_PHY_FIBERENGINE_H_ */
