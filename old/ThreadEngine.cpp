/*
 * ThreadEngine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include <condition_variable>

#include "phy/ThreadEngine.h"

namespace vnl { namespace phy {

class ThreadEngine::Thread {
public:
	Thread(ThreadEngine* engine) : engine(engine), ulock(engine->mutex), thread(0) {
		ulock.unlock();
	}
	
	virtual ~Thread() {
		docancel = true;
		notify();
		engine->mutex.unlock();
		thread->join();
		delete thread;
		engine->mutex.lock();
	}
	
	void start() {
		thread = new std::thread(&Thread::entry, this);
	}
	
	virtual void run() = 0;
	
	bool poll(int millis) {
		bool res = cond.wait_for(ulock, std::chrono::milliseconds(millis)) == std::cv_status::no_timeout;
		engine->current = this;
		return res;
	}
	
	void notify() {
		cond.notify_all();
	}
	
	void sent(Message* msg) {
		pending++;
		if(!msg->async && pending > 0) {
			wait();
		}
	}
	
	void acked() {
		pending--;
		if(pending == 0) {
			notify();
		}
	}
	
	void flush() {
		while(pending) {
			wait();
		}
	}
	
protected:
	void wait() {
		engine->current = 0;
		cond.wait(ulock);
		if(docancel) {
			throw cancel_t();
		}
		engine->current = this;
	}
	
	void entry() {
		run();
	}
	
	ThreadEngine* engine;
	std::unique_lock<std::recursive_mutex> ulock;
	std::condition_variable_any cond;
	std::thread* thread;
	int pending = 0;
	bool docancel = false;
	
};

class ThreadEngine::Worker : public Thread {
public:
	Worker(ThreadEngine* engine) : Thread(engine) {}
	
	void run() {
		ulock.lock();
		engine->current = this;
		Message* msg = 0;
		try {
			while(true) {
				if(msg) {
					engine->dispatch(msg);
				}
				while(!queue.pop(msg)) {
					wait();
				}
			}
		} catch(cancel_t&) {}
		ulock.unlock();
	}
	
	void push(Message* msg) {
		queue.push(msg);
		cond.notify_all();
	}
	
protected:
	vnl::util::simple_queue<Message*> queue;
	
};

class ThreadEngine::Task : public Thread {
public:
	Task(ThreadEngine* engine) : Thread(engine), tid(0), task(0) {}
	
	~Task() {
		delete task;
	}
	
	void run() {
		ulock.lock();
		engine->current = this;
		try {
			while(true) {
				task->run();
				delete task;
				task = 0;
				engine->enqueue(this);
				wait();
			}
		} catch(cancel_t&) {}
		ulock.unlock();
	}
	
	uint64_t launch(Runnable* task_) {
		tid = engine->rand();
		task = task_;
		notify();
		return tid;
	}
	
	uint64_t tid;
	Runnable* task;
	
};


ThreadEngine::ThreadEngine(int N) : N(N) {
	workers = new Worker*[N];
	for(int i = 0; i < N; ++i) {
		workers[i] = new Worker(this);
		workers[i]->start();
	}
}

ThreadEngine::~ThreadEngine() {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	dorun = false;
	for(auto task : finished) {
		delete task;
	}
	for(int i = 0; i < N; ++i) {
		delete workers[i];
	}
	delete [] workers;
}

void ThreadEngine::send(Message* msg) {
	if(current) {
		msg->impl = current;
		forward(msg);
		current->sent(msg);
	}
}

void ThreadEngine::flush() {
	if(current) {
		current->flush();
	}
}

void ThreadEngine::handle(Message* msg, Stream* stream) {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	if(msg->isack) {
		Thread* thread = (Thread*)msg->impl;
		if(msg->callback) {
			msg->callback(msg);
		}
		thread->acked();
		return;
	} else if(stream) {
		dispatch(msg, stream);
		auto iter = polling.find(msg->sid);
		if(iter != polling.end() && iter->second.size()) {
			auto& queue = iter->second;
			auto fiber = queue.begin();
			(*fiber)->notify();
			queue.erase(fiber);
		}
		return;
	}
	workers[msg->dst->mac % N]->push(msg);
}

void ThreadEngine::open(Stream* stream) {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	polling[stream->sid];
}

void ThreadEngine::close(Stream* stream) {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	polling.erase(stream->sid);
}

bool ThreadEngine::poll(Stream* stream, int millis) {
	if(current) {
		polling[stream->sid].push_back(current);
		return current->poll(millis);
	}
	return false;
}

uint64_t ThreadEngine::launch(Runnable* task_) {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	Task* task;
	if(finished.empty()) {
		task = new Task(this);
		task->start();
	} else {
		task = finished.back();
		finished.pop_back();
	}
	uint64_t tid = task->launch(task_);
	tasks[tid] = task;
	return tid;
}

void ThreadEngine::cancel(uint64_t tid) {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	auto iter = tasks.find(tid);
	if(iter != tasks.end()) {
		delete iter->second;
		tasks.erase(iter);
	}
}

int ThreadEngine::timeout() {
	return 1000;
}

void ThreadEngine::enqueue(Task* task) {
	tasks.erase(task->tid);
	finished.push_back(task);
}


}}
