/*
 * ThreadEngine.h
 *
 *  Created on: Jan 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_THREADENGINE_H_
#define INCLUDE_PHY_THREADENGINE_H_

#include <mutex>
#include <condition_variable>

#include "Object.h"

namespace vnl { namespace phy {

class ThreadEngine : public vnl::phy::Engine {
public:
	ThreadEngine(int N) : N(N) {
		workers = new Worker*[N];
		for(int i = 0; i < N; ++i) {
			workers[i] = new Worker(this);
			workers[i]->start();
		}
	}
	
	~ThreadEngine() {
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
	
	void send(Message* msg) override {
		if(current) {
			msg->impl = current;
			forward(msg);
			current->sent(msg);
		}
	}
	
	void flush() override {
		if(current) { current->flush(); }
	}
	
	void handle(Message* msg) override {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if(msg->isack) {
			((Thread*)msg->impl)->acked();
			return;
		} else if(msg->sid) {
			Stream* stream = get_stream(msg->dst, msg->sid);
			if(stream) {
				dispatch(msg, stream);
				Thread* fiber;
				auto* queue = polling.get(msg->sid);
				if(queue && queue->pop(fiber)) {
					fiber->notify();
				}
				return;
			}
		}
		Worker* fiber;
		if(!avail.empty()) {
			fiber = avail.top(); avail.pop();
		} else if(msg->src) {
			fiber = workers[msg->src->getMAC() % N];
		} else {
			fiber = workers[0];
		}
		fiber->push(msg);
	}
	
	void open(Stream* stream) override {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		polling.put(stream->sid);
	}
	
	void close(Stream* stream) override {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		polling.erase(stream->sid);
	}
	
	bool poll(Stream* stream, int millis) override {
		if(current) {
			polling.get(stream->sid)->push(current);
			return current->poll(millis);
		}
		return false;
	}
	
	uint64_t launch(Runnable* task) override {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Task* fiber;
		if(!finished.pop(fiber)) {
			fiber = new Task(this);
			fiber->start();
		}
		uint64_t tid = fiber->launch(task);
		tasks.put(tid, fiber);
		return tid;
	}
	
	void cancel(uint64_t tid) override {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Task** fiber = tasks.get(tid);
		if(fiber) {
			delete *fiber;
			tasks.erase(tid);
		}
	}
	
	int timeout() override {
		return 1000;
	}
	
protected:
	class cancel_t {};
	
	class Thread {
	public:
    	Thread(ThreadEngine* engine)
			: 	engine(engine), ulock(engine->mutex), thread(0)
		{
    		ulock.unlock();
    	}
    	~Thread() {
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
	
	class Worker : public Thread {
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
						engine->enqueue(this);
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
	
	class Task : public Thread {
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
	
	void enqueue(Worker* fiber) {
		avail.push(fiber);
	}
	
	void enqueue(Task* task) {
		tasks.erase(task->tid);
		finished.push(task);
	}
	
private:
	int N;
	bool dorun = true;
	std::recursive_mutex mutex;
	Thread* current = 0;
	Worker** workers;
	vnl::util::simple_stack<Worker*> avail;
	vnl::util::simple_hashmap<uint64_t, vnl::util::simple_queue<Thread*> > polling;
	vnl::util::simple_hashmap<uint64_t, Task*> tasks;
	vnl::util::simple_queue<Task*> finished;
	
};

}}

#endif /* INCLUDE_PHY_THREADENGINE_H_ */
