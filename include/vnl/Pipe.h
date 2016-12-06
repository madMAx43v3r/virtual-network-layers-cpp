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
	
	static Pipe* create(Basic* target) {
		std::lock_guard<std::mutex> lock(sync);
		Pipe* pipe = pool.create();
		pipe->target = target;
		num_open++;
		return pipe;
	}
	
	static Pipe* create() {
		std::lock_guard<std::mutex> lock(sync);
		num_open++;
		return pool.create();
	}
	
	static int get_num_open() {
		std::lock_guard<std::mutex> lock(sync);
		return num_open;
	}
	
	Pipe* open(Basic* target) {
		std::lock_guard<std::mutex> lock(mutex);
		this->target = target;
		count++;
		return this;
	}
	
	Pipe* attach() {
		std::lock_guard<std::mutex> lock(mutex);
		count++;
		return this;
	}
	
	void detach() {
		std::lock_guard<std::mutex> lock(mutex);
		dec();
	}
	
	void close() {
		std::lock_guard<std::mutex> lock(mutex);
		target = 0;
		dec();
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
	void dec() {
		assert(count > 0);
		if(--count == 0) {
			assert(target == 0);
			std::lock_guard<std::mutex> lock(sync);
			pool.destroy(this);
			num_open--;
		}
	}
	
private:
	Basic* target;
	int count;
	
	static int num_open;
	static Pool<Pipe> pool;
	static std::mutex sync;
	
};


} // vnl

#endif /* INCLUDE_VNL_PIPE_H_ */
