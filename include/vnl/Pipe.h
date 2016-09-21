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
	Pipe(Basic* target) : target(target) {
		connect_t msg(this);
		vnl::send(&msg, target);
	}
	
	~Pipe() {
		close_t msg(this);
		vnl::send(&msg, target);
	}
	
	typedef MessageType<Basic*, 0xd8577f3e> connect_t;
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
	Basic* target;
	
};


}

#endif /* INCLUDE_VNL_PIPE_H_ */
