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
		thread = new std::thread(&Engine::entry, this);
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
