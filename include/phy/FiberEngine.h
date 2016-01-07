/*
 * FiberEngine.h
 *
 *  Created on: Dec 17, 2015
 *      Author: mad
 */

#ifndef INCLUDE_PHY_FIBERENGINE_H_
#define INCLUDE_PHY_FIBERENGINE_H_

#include "Engine.h"
#include "util/simple_stack.h"
#include "util/simple_queue.h"
#include "util/simple_hashmap.h"

namespace vnl { namespace phy {

class FiberEngine : public Engine {
public:
	FiberEngine(int N);
	~FiberEngine();
	
	void send(Message* msg) override;
	void flush() override;
	void handle(Message* msg) override;
	void open(Stream* stream) override;
	void close(Stream* stream) override;
	bool poll(Stream* stream, int millis) override;
	uint64_t launch(Runnable* task) override;
	void cancel(uint64_t tid) override;
	int timeout() override;
	
protected:
	class Fiber;
	class Worker;
	class Task;
	
	void enqueue(Worker* fiber);
	void enqueue(Task* task);
	
private:
	int N;
	Fiber* current = 0;
	Worker** workers;
	vnl::util::simple_stack<Worker*> avail;
	vnl::util::simple_hashmap<uint64_t, vnl::util::simple_queue<Fiber*> > polling;
	vnl::util::simple_hashmap<uint64_t, Task*> tasks;
	vnl::util::simple_queue<Task*> finished;
	
};

}}

#endif /* INCLUDE_PHY_FIBERENGINE_H_ */
