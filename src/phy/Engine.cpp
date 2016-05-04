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
#include "phy/Registry.h"


namespace vnl { namespace phy {

thread_local Engine* Engine::local = 0;

Engine::Engine(uint64_t mac)
	:	Object(mac),
	 	ulock(mutex), queue(&memory), buffer(&memory)
{
	assert(Engine::local == 0);
	Engine::local = this;
	ulock.unlock();
	static std::atomic<int> counter;
	generator.seed(Util::hash64(counter++, (uint64_t)std::hash(std::this_thread::get_id()), System::nanoTime()));
	async_cb = std::bind(&Engine::async_ack, this, std::placeholders::_1);
}

Engine::~Engine() {
	Engine::local = 0;
}

void Engine::exec(Object* object) {
	object->run(this);
	send_async(Registry::finished_t(object), Registry::instance);
}

void Engine::send_impl(Message* msg, Node* dst, bool async) {
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
	assert(stream->engine == this);
	assert(current);
	
	stream->impl = current;
	return current->poll(millis);
}

void Engine::flush() {
	assert(Engine::local == this);
	current->flush();
}

Message* Engine::collect(int64_t timeout) {
	Message* msg = 0;
	sync.lock();
	if(queue.pop(msg)) {
		sync.unlock();
		return msg;
	} else {
		sync.unlock();
	}
	if(timeout >= 0) {
		lock();
		waiting.store(1, std::memory_order_release);
		sync.lock();
		if(!queue.pop(msg)) {
			sync.unlock();
			wait(timeout);
		} else {
			sync.unlock();
		}
		waiting.store(0, std::memory_order_release);
		unlock();
		sync.lock();
		queue.pop(msg);
		sync.unlock();
	}
	return msg;
}






}}
