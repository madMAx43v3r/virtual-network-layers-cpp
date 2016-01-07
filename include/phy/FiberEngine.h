/*
 * FiberEngine.h
 *
 *  Created on: Dec 17, 2015
 *      Author: mad
 */

#ifndef INCLUDE_PHY_FIBERENGINE_H_
#define INCLUDE_PHY_FIBERENGINE_H_

#include <chrono>

#include <boost/bind.hpp>
#include <boost/coroutine/symmetric_coroutine.hpp>

#include "util/simple_stack.h"
#include "util/simple_queue.h"
#include "util/simple_hashmap.h"

namespace vnl { namespace phy {

class FiberEngine : public Engine {
public:
	FiberEngine(int N) : N(N) {
		workers = new Worker*[N];
		for(int i = 0; i < N; ++i) {
			workers[i] = new Worker(this);
			workers[i]->start();
		}
	}
	
	~FiberEngine() {
		for(auto task : tasks) {
			delete task.val;
		}
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
		if(msg->isack) {
			((Fiber*)msg->impl)->acked();
			return;
		} else if(msg->sid) {
			Stream* stream = get_stream(msg->dst, msg->sid);
			if(stream) {
				dispatch(msg, stream);
				Fiber* fiber;
				auto* queue = polling.get(msg->sid);
				if(queue && queue->pop(fiber)) {
					fiber->notify(true);
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
		polling.put(stream->sid);
	}
	
	void close(Stream* stream) override {
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
		Task* fiber;
		if(!finished.pop(fiber)) {
			fiber = new Task(this);
			fiber->start();
		}
		uint64_t tid = fiber->launch(task);
		if(fiber->task) {
			tasks.put(tid, fiber);
		}
		return tid;
	}
	
	void cancel(uint64_t tid) override {
		Task** fiber = tasks.get(tid);
		if(fiber) {
			delete *fiber;
			tasks.erase(tid);
		}
	}
	
	int timeout() override {
		int64_t now = System::currentTimeMillis();
		std::vector<Fiber*> list;
		int millis = 1000;
		while(true) {
			for(auto& stream : polling) {
				auto& queue = stream.val;
				for(auto iter = queue.begin(); iter != queue.end(); ++iter) {
					Fiber* fiber = *iter;
					int diff = fiber->timeout - now;
					if(diff <= 0) {
						list.push_back(fiber);
						queue.erase(iter);
					} else if(diff < millis) {
						millis = diff;
					}
				}
			}
			if(list.size()) {
				for(Fiber* fiber : list) {
					fiber->notify(false);
				}
				list.clear();
			} else {
				break;
			}
		}
		return millis;
	}
	
protected:
	class Fiber {
	public:
		typedef boost::coroutines::symmetric_coroutine<void> fiber;
    	Fiber(FiberEngine* engine)
			: 	engine(engine), _call(boost::bind(&Fiber::entry, this, boost::arg<1>())) {}
		virtual void run() = 0;
		void start() { call(); }
		bool poll(int millis) {
			timeout = System::currentTimeMillis() + millis;
			poll();
			return result;
		}
		void notify(bool res) {
			if(polling) {
				result = res;
				call();
			}
		}
		void sent(Message* msg) {
			pending++;
			if(!msg->async && pending > 0) {
				wait();
			}
		}
		void acked() {
			pending--;
			if(pending == 0 && waiting) {
				call();
			}
		}
		void flush() {
			while(pending) {
				wait();
			}
		}
		int64_t timeout = 0;
		bool result = false;
	protected:
		void poll() {
			polling = true;
			yield();
			polling = false;
		}
		void wait() {
			waiting = true;
			yield();
			waiting = false;
		}
		void call() {
			Fiber* tmp = engine->current;
			engine->current = this;
			_call();
			engine->current = tmp;
		}
		void yield() {
			(*_yield)();
		}
		void entry(fiber::yield_type& yield_) {
    		_yield = &yield_;
    		run();
    	}
		FiberEngine* engine;
		fiber::call_type _call;
		fiber::yield_type* _yield = 0;
		int pending = 0;
		bool polling = false;
		bool waiting = false;
	};
	
	class Worker : public Fiber {
	public:
		Worker(FiberEngine* engine) : Fiber(engine) {}
		void run() {
			while(true) {
				Message* msg;
				while(!queue.pop(msg)) {
					engine->enqueue(this);
					idle = true;
					yield();
					idle = false;
				}
				engine->dispatch(msg);
			}
		}
		void push(Message* msg) {
			queue.push(msg);
			if(idle) {
				call();
			}
		}
	protected:
		vnl::util::simple_queue<Message*> queue;
		bool idle = false;
	};
	
	class Task : public Fiber {
	public:
		Task(FiberEngine* engine) : Fiber(engine), tid(0), task(0) {}
		~Task() {
			delete task;
		}
		void run() {
			while(true) {
				yield();
				task->run();
				delete task;
				task = 0;
				engine->enqueue(this);
			}
		}
		uint64_t launch(Runnable* task_) {
			tid = engine->rand();
			task = task_;
			call();
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
	Fiber* current = 0;
	Worker** workers;
	vnl::util::simple_stack<Worker*> avail;
	vnl::util::simple_hashmap<uint64_t, vnl::util::simple_queue<Fiber*> > polling;
	vnl::util::simple_hashmap<uint64_t, Task*> tasks;
	vnl::util::simple_queue<Task*> finished;
	
	
};

}}

#endif /* INCLUDE_PHY_FIBERENGINE_H_ */
