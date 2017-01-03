/*
 * Engine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include <assert.h>
#include <string.h>

#include <vnl/Engine.h>
#include <vnl/Object.h>
#include <vnl/Random.h>
#include <vnl/Exception.hxx>


namespace vnl {

List<Engine*>* Engine::instances = 0;
std::mutex Engine::static_mutex;

void Engine::exec(Object* object, Message* init, Pipe* pipe) {
	assert(object->vnl_dorun == false);
	
	try {
		object->exec(this, init, pipe);
	} catch (const vnl::Exception& ex) {
		std::cout << "ERROR: " << object->my_topic << ": Caught " << ex.get_type_name() << std::endl;
	} catch (const std::exception& ex) {
		std::cout << "ERROR: " << object->my_topic << ": Caught " << ex.what() << std::endl;
	} catch (...) {
		std::cout << "ERROR: " << object->my_topic << ": Caught unknown exeption!" << std::endl;
	}
	
	flush();
	delete object;
	
	while(true) {
		Message* msg = collect(0);
		if(msg) {
			msg->ack();
		} else {
			break;
		}
	}
}

Message* Engine::collect(int64_t timeout) {
	std::unique_lock<std::mutex> ulock(mutex);
	Message* msg = 0;
	if(queue.pop(msg)) {
		return msg;
	}
	if(timeout != 0) {
		if(timeout > 0) {
			cond.wait_for(ulock, std::chrono::microseconds(timeout));
		} else {
			cond.wait(ulock);
		}
		queue.pop(msg);
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
	return count;
}



}
