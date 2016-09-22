/*
 * Pipe.h
 *
 *  Created on: Sep 20, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_PIPE_H_
#define INCLUDE_VNL_PIPE_H_

#include <vnl/Basic.h>
#include <vnl/Queue.h>


namespace vnl {

class Pipe : public Reactor {
public:
	Pipe() : target(0) {}
	
	Pipe(const Pipe&) = delete;
	Pipe& operator=(const Pipe&) = delete;
	
	~Pipe() {
		assert(target == 0);
	}
	
	// thread safe
	void open() {
		open_t msg;
		vnl::send(&msg, this);
	}
	
	// thread safe
	void close() {
		wait_t msg;
		vnl::send(&msg, this);
	}
	
	// NOT thread safe (only target is allowed)
	void ack(Basic* dst) {
		lock();
		assert(target == 0);
		target = dst;
		Message* wait;
		while(waitlist.pop(wait)) {
			wait->ack();
		}
		unlock();
	}
	
	// NOT thread safe (only target is allowed)
	void fin() {
		lock();
		target = 0;
		unlock();
	}
	
	typedef SignalType<0xcb1b0f44> open_t;
	typedef SignalType<0x1a740a38> wait_t;
	
	typedef RequestType<bool, Basic*, 0xd8577f3e> connect_t;
	typedef MessageType<Basic*, 0x7ddae559> close_t;
	
protected:
	bool handle(Message* msg) {
		if(msg->msg_id == open_t::MID) {
			if(!target) {
				waitlist.push(msg);
				return true;
			}
		} else if(msg->msg_id == wait_t::MID) {
			if(target) {
				waitlist.push(msg);
				return true;
			}
		} else if(msg->msg_id == close_t::MID) {
			if(((close_t*)msg)->data == target) {
				Message* wait;
				while(waitlist.pop(wait)) {
					wait->ack();
				}
				target = 0;
			}
		} else if(target) {
			msg->dst = target;
			target->receive(msg);
			return true;
		}
		return false;
	}
	
private:
	Basic* target;
	
	Queue<Message*> waitlist;
	
};


}

#endif /* INCLUDE_VNL_PIPE_H_ */
