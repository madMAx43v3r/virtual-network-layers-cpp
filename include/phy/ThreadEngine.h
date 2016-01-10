/*
 * ThreadEngine.h
 *
 *  Created on: Jan 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_THREADENGINE_H_
#define INCLUDE_PHY_THREADENGINE_H_

#include <vector>
#include <unordered_map>
#include <mutex>

#include "Engine.h"

namespace vnl { namespace phy {

class ThreadEngine : public vnl::phy::Engine {
public:
	ThreadEngine(int N);
	~ThreadEngine();
	
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
	class cancel_t {};
	class Thread;
	class Worker;
	class Task;
	
	void enqueue(Task* task);
	
private:
	int N;
	bool dorun = true;
	std::recursive_mutex mutex;
	Thread* current = 0;
	Worker** workers;
	std::vector<Worker*> avail;
	std::vector<Task*> finished;
	std::unordered_map<uint64_t, std::vector<Thread*> > polling;
	std::unordered_map<uint64_t, Task*> tasks;
	
};


}}

#endif /* INCLUDE_PHY_THREADENGINE_H_ */
