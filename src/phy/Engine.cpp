/*
 * Engine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include <assert.h>

#include "phy/Engine.h"
#include "phy/Stream.h"
#include "phy/Fiber.h"

namespace vnl { namespace phy {

thread_local Engine* Engine::local = 0;

Engine::Engine() : ulock(mutex), thread(0) {
	ulock.unlock();
	static std::atomic<int> counter;
	generator.seed(Util::hash64(counter++, System::nanoTime()));
	acks.reserve(1024);
	queue.reserve(1024);
	fibers.reserve(1024);
	avail.reserve(1024);
	pages.reserve(1024);
}

Engine::~Engine() {
	for(Fiber* fiber : fibers) {
		delete fiber;
	}
	for(Page* page : pages) {
		delete page;
	}
}

void Engine::start() {
	if(!thread) {
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

void Engine::send(Message* msg, Stream* dst, bool async) {
	assert(msg->impl == 0);
	assert(msg->isack == false);
	assert(current);
	msg->dst = dst;
	msg->async = async;
	msg->impl = current;
	dst->engine->receive(msg);
	current->sent(msg);
}

bool Engine::poll(Stream* stream, int millis) {
	stream->impl.push(current);
	bool res = current->poll(millis);
	return res;
}

void Engine::flush() {
	current->flush();
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

Page* Engine::get_page() {
	Page* page;
	if(pages.size()) {
		page = pages.back();
		pages.pop_back();
	} else {
		page = new Page(this);
	}
	return page;
}

void Engine::free_page(Page* page) {
	if(pages.size() < 1024) {
		pages.push_back(page);
	} else {
		delete page;
	}
}



}}
