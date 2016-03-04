/*
 * Engine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include "phy/Engine.h"

namespace vnl { namespace phy {

thread_local Engine* Engine::local = 0;

Engine::Engine() : ulock(mutex), thread(0), core_id(-1) {
	static std::atomic<int> counter;
	generator.seed(Util::hash64(counter++, System::nanoTime()));
	ulock.unlock();
}

Engine::~Engine() {
	for(Fiber* fiber : fibers) {
		delete fiber;
	}
}

void Engine::start(int core) {
	if(!thread) {
		core_id = core;
		lock();
		thread = new std::thread(&Engine::mainloop, this);
		while(!dorun) {
			wait();
		}
		unlock();
	}
}

void Engine::stop() {
	dorun = false;
	if(thread) {
		lock();
		notify();
		unlock();
		thread->join();
		delete thread;
		thread = 0;
	}
}

void Engine::mainloop() {
	local = this;
	if(core_id >= 0) {
		Util::stick_to_core(core_id);
	}
	impl_lock();
	run();
	impl_unlock();
	dorun = true;
	notify();
	std::vector<Stream*> pending;
	std::vector<Message*> inbox;
	while(dorun) {
		inbox.clear();
		while(dorun) {
			int to = timeout();
			lock();
			if(acks.empty() && queue.empty()) {
				wait(to);
			} else {
				Message* msg;
				while(acks.pop(msg)) {
					inbox.push_back(msg);
				}
				while(queue.pop(msg)) {
					inbox.push_back(msg);
				}
				unlock();
				break;
			}
			unlock();
		}
		pending.clear();
		impl_lock();
		for(Message* msg : inbox) {
			if(debug > 0) {
				std::cout << std::dec << System::currentTimeMillis() << " Engine@" << this << " " << (msg->isack ? "ACK" : "RCV") << " " << msg->toString() << std::endl;
			}
			if(msg->isack) {
				msg->impl->acked(msg);
			} else {
				Stream* stream = msg->dst->get_stream(msg->sid);
				if(stream) {
					stream->push(msg);
					pending.push_back(stream);
				}
			}
		}
		for(Stream* stream : pending) {
			if(stream->queue.size()) {
				if(stream->sid == 0) {
					stream->obj->process();
				}
				Fiber* fiber;
				if(stream->impl.pop(fiber)) {
					fiber->notify(true);
				}
			}
		}
		impl_unlock();
	}
	impl_lock();
	for(Fiber* fiber : fibers) {
		fiber->stop();
	}
	impl_unlock();
}

taskid_t Engine::launch(const std::function<void()>& func) {
	Fiber* fiber;
	if(avail.empty()) {
		fiber = create();
		fiber->start();
		fibers.insert(fiber);
	} else {
		fiber = avail.back();
		avail.pop_back();
	}
	taskid_t task;
	task.id = nextid++;
	task.impl = fiber;
	fiber->launch(func, task.id);
	return task;
}

void Engine::cancel(taskid_t task) {
	if(task.impl) {
		Fiber* fiber = task.impl;
		if(fiber->tid == task.id) {
			fiber->stop();
			fibers.erase(fiber);
			delete fiber;
		}
	}
}





}}
