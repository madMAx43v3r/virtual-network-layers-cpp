/*
 * Pipe.h
 *
 *  Created on: Sep 20, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_PIPE_H_
#define INCLUDE_VNL_PIPE_H_

#include <vnl/Reactor.h>
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
	
	typedef MessageType<Pipe*, 0x87978b67> connect_t;		// like TCP SYN
	typedef SignalType<0xcb1b0f44> open_t;					// like TCP wait for SYN ACK
	typedef RequestType<bool, Basic*, 0xd8577f3e> attach_t;	// like TCP SYN ACK
	typedef MessageType<Pipe*, 0x7ddae559> close_t;			// like TCP FIN
	typedef SignalType<0x1a740a38> wait_t;					// like TCP wait for FIN ACK
	typedef MessageType<Basic*, 0x528852ed> reset_t;		// like TCP connection reset
	
protected:
	bool handle(Message* msg) {
		if(msg->msg_id == attach_t::MID) {
			attach_t* request = (attach_t*)msg;
			if(!target) {
				target = request->args;
				notify_all();
				request->res = true;
			} else {
				request->res = false;
			}
		} else if(msg->msg_id == open_t::MID) {
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
			if(target) {
				((close_t*)msg)->data = this;
				forward(msg);
				notify_all();
				target = 0;
				return true;
			}
		} else if(msg->msg_id == reset_t::MID) {
			assert(((reset_t*)msg)->data == target);
			notify_all();
			target = 0;
		} else if(target) {
			forward(msg);
			return true;
		}
		return false;
	}
	
	void forward(Message* msg) {
		msg->dst = target;
		target->receive(msg);
	}
	
	void notify_all() {
		Message* wait;
		while(waitlist.pop(wait)) {
			wait->ack();
		}
	}
	
private:
	Basic* target;
	
	Queue<Message*> waitlist;
	
};


}

#endif /* INCLUDE_VNL_PIPE_H_ */
