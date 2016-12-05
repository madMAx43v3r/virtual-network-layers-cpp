/*
 * Reactor.h
 *
 *  Created on: Dec 1, 2016
 *      Author: mad
 */

#ifndef INCLUDE_REACTOR_H_
#define INCLUDE_REACTOR_H_

#include <mutex>

#include <vnl/Basic.h>
#include <vnl/Message.h>


namespace vnl {

class Reactor : public Node {
public:
	Reactor() {}
	
	virtual void receive(Message* msg) {
		msg->gate = this;
		mutex.lock();
		if(msg->isack) {
			callback(msg);
			msg->destroy();
		} else {
			if(!handle(msg)) {
				msg->ack();
			}
		}
		mutex.unlock();
	}
	
protected:
	virtual bool handle(Message* msg) = 0;
	
	virtual void callback(Message* msg) {}
	
	void send_async(Message* msg, Basic* dst) {
		msg->src = this;
		dst->receive(msg);
	}
	
	void lock() {
		mutex.lock();
	}
	
	void unlock() {
		mutex.unlock();
	}
	
private:
	std::mutex mutex;
	
};


} // vnl

#endif /* INCLUDE_REACTOR_H_ */
