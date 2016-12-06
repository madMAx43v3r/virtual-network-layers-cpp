/*
 * Actor.h
 *
 *  Created on: Dec 1, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_ACTOR_H_
#define INCLUDE_VNL_ACTOR_H_

#include <mutex>
#include <condition_variable>

#include <vnl/Basic.h>
#include <vnl/Message.h>
#include <vnl/Pipe.h>


namespace vnl {

class Actor : public Node {
public:
	Actor() : ulock(mutex) {}
	
	virtual void receive(Message* msg) {
		if(msg->isack) {
			std::unique_lock<std::mutex> lock(mutex);
			acked = true;
			cond.notify_all();
		}
	}
	
	void reset() {
		acked = false;
	}
	
	void send(Message* msg, Basic* dst) {
		msg->src = this;
		msg->dst = dst;
		acked = false;
		ulock.unlock();
		dst->receive(msg);
		ulock.lock();
		while(!acked) {
			cond.wait(ulock);
		}
	}
	
	void wait_for_ack() {
		while(!acked) {
			cond.wait(ulock);
		}
	}
	
private:
	std::mutex mutex;
	std::condition_variable cond;
	std::unique_lock<std::mutex> ulock;
	
	bool acked = false;
	
};


} // vnl

#endif /* INCLUDE_VNL_ACTOR_H_ */
