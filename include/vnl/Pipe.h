/*
 * Pipe.h
 *
 *  Created on: Sep 20, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_PIPE_H_
#define INCLUDE_VNL_PIPE_H_

#include <vnl/Reactor.h>
#include <vnl/Pool.h>


namespace vnl {

class Pipe : public Reactor {
public:
	Pipe() : target(0), count(1) {}
	
	static Pipe* open() {
		std::lock_guard<std::mutex> lock(sync);
		return pool.create();
	}
	
	void attach(Basic* target) {
		std::lock_guard<std::mutex> lock(mutex);
		this->target = target;
		count++;
	}
	
	void close() {
		mutex.lock();
		assert(count > 0);
		target = 0;
		if(--count == 0) {
			mutex.unlock();
			std::lock_guard<std::mutex> lock(sync);
			pool.destroy(this);
		} else {
			mutex.unlock();
		}
	}
	
protected:
	bool handle(Message* msg) {
		if(target) {
			msg->dst = target;
			target->receive(msg);
			return true;
		}
		return false;
	}
	
private:
	Basic* target;
	int count;
	
	static Pool<Pipe> pool;
	static std::mutex sync;
	
};


} // vnl

#endif /* INCLUDE_VNL_PIPE_H_ */
