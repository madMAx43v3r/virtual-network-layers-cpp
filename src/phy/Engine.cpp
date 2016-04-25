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
#include "phy/Object.h"
#include "phy/Fiber.h"

namespace vnl { namespace phy {

thread_local Engine* Engine::local = 0;

Engine::Engine() : ulock(mutex), thread(0) {
	ulock.unlock();
	static std::atomic<int> counter;
	generator.seed(Util::hash64(counter++, (uint64_t)std::hash(std::this_thread::get_id()), System::nanoTime()));
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
	}
}

void Engine::send(Message* msg, Node* dst, bool async) {
	assert(Engine::local == this);
	assert(msg->impl == 0);
	assert(msg->isack == false);
	assert(dst);
	assert(current);
	msg->src = this;
	msg->impl = current;
	dst->receive(msg);
	current->sent(msg, async);
}

bool Engine::poll(Stream* stream, int64_t millis) {
	assert(Engine::local == this);
	stream->impl.push(current);
	return current->poll(millis);
}

void Engine::flush() {
	assert(Engine::local == this);
	current->flush();
}

taskid_t Engine::launch(const std::function<void()>& func) {
	assert(Engine::local == this);
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

bool Engine::listen_on(const taskid_t& task, Stream* dst) {
	assert(Engine::local == this);
	Fiber* fiber = task.impl;
	if(fiber->task.id == task.id) {
		fiber->waitlist.push_back(dst);
		return true;
	}
	return false;
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
		page->next = 0;
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


void Fiber::finished(Engine* engine, Fiber* fiber) {
	for(Stream* stream : waitlist) {
		stream->receive(new Engine::finished_t(task.id, true));
	}
	task.id = 0;
	waitlist.clear();
	engine->finished(fiber);
}



}}
