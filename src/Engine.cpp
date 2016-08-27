/*
 * Engine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include <assert.h>
#include <string.h>

#include "vnl/Engine.h"
#include "vnl/Object.h"
#include "vnl/Random.h"


namespace vnl {

Engine::Engine()
	:	num_queued(0)
{
	mac = Random64::global_rand();
}

void Engine::exec(Object* object, Message* init) {
	object->exec(this, init);
	delete object;
}

Message* Engine::collect(int64_t timeout) {
	std::unique_lock<std::mutex> ulock(mutex);
	Message* msg = 0;
	if(queue.pop(msg)) {
		num_queued--;
		return msg;
	}
	if(timeout != 0) {
		if(timeout > 0) {
			cond.wait_for(ulock, std::chrono::microseconds(timeout));
		} else {
			cond.wait(ulock);
		}
		if(queue.pop(msg)) {
			num_queued--;
		}
	}
	return msg;
}

size_t Engine::collect(int64_t timeout, vnl::Queue<Message*>& inbox) {
	std::unique_lock<std::mutex> ulock(mutex);
	size_t count = 0;
	Message* msg = 0;
	while(queue.pop(msg)) {
		inbox.push(msg);
		count++;
	}
	if(!count && timeout != 0) {
		if(timeout > 0) {
			cond.wait_for(ulock, std::chrono::microseconds(timeout));
		} else {
			cond.wait(ulock);
		}
		while(queue.pop(msg)) {
			inbox.push(msg);
			count++;
		}
	}
	num_queued -= count;
	return count;
}



}
