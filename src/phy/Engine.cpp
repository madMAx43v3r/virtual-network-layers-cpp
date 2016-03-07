/*
 * Engine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include <assert.h>
#include <string.h>

#include "phy/Engine.h"
#include "phy/Stream.h"
#include "phy/Fiber.h"

namespace vnl { namespace phy {

thread_local Engine* Engine::local = 0;

Engine::Engine() : ulock(mutex), thread(0) {
	ulock.unlock();
	static std::atomic<int> counter;
	generator.seed(Util::hash64(counter++, System::nanoTime()));
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
		thread = new std::thread(&Engine::entry, this);
	}
}

void Engine::stop() {
	if(thread) {
		lock();
		dorun = false;
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
	return current->poll(millis);
}

void Engine::flush() {
	current->flush();
}

taskid_t Engine::launch(const std::function<void()>& func) {
	Fiber* fiber;
	if(avail.empty()) {
		fiber = create();
		fiber->start();
		fibers.push_back(fiber);
	} else {
		fiber = avail.back();
		avail.pop_back();
	}
	taskid_t task;
	task.id = nextid++;
	task.impl = fiber;
	task.func = func;
	fiber->launch(task);
	return task;
}

int Engine::collect(std::vector<Message*>& inbox, int timeout) {
	inbox.clear();
	Message* msg = 0;
	while(queue.pop(msg)) {
		inbox.push_back(msg);
	}
	int res = inbox.size();
	if(!res) {
		lock();
		waiting.store(1, std::memory_order_release);
		if(queue.pop(msg)) {
			inbox.push_back(msg);
		} else {
			wait(timeout);
		}
		waiting.store(0, std::memory_order_release);
		unlock();
		while(queue.pop(msg)) {
			inbox.push_back(msg);
		}
		res = inbox.size();
	}
	return res;
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
