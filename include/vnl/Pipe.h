/*
 * Pipe.h
 *
 *  Created on: Sep 20, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_PIPE_H_
#define INCLUDE_VNL_PIPE_H_

#include <vnl/Basic.h>


namespace vnl {

class Pipe : public Reactor {
public:
	Pipe() : target(0) {}
	
	Pipe(const Pipe&) = delete;
	Pipe& operator=(const Pipe&) = delete;
	
	~Pipe() {
		close();
	}
	
	void ack(Basic* dst) {
		lock();
		close();
		target = dst;
		unlock();
	}
	
	void fin() {
		lock();
		target = 0;
		unlock();
	}
	
	typedef RequestType<bool, Basic*, 0xd8577f3e> connect_t;
	typedef MessageType<Basic*, 0x7ddae559> close_t;
	
protected:
	bool handle(Message* msg) {
		if(msg->msg_id == close_t::MID) {
			if(((close_t*)msg)->data == target) {
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
	void close() {
		if(target) {
			close_t msg(this);
			vnl::send(&msg, target);
			target = 0;
		}
	}
	
private:
	Basic* target;
	
};


}

#endif /* INCLUDE_VNL_PIPE_H_ */
