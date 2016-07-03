/*
 * Node.h
 *
 *  Created on: Apr 25, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_NODE_H_
#define INCLUDE_PHY_NODE_H_

#include <mutex>
#include <condition_variable>

#include "vnl/Random.h"
#include "vnl/Message.h"


namespace vnl {

class Basic {
public:
	Basic() {}
	
	Basic(const Basic&) = delete;
	Basic& operator=(const Basic&) = delete;
	
	virtual ~Basic() {}
	
	uint64_t get_mac() const { return mac; }
	
	// must be thread safe !!!
	virtual void receive(Message* msg) = 0;
	
	void* _impl = 0;
	
protected:
	uint64_t mac = 0;
	
};


class Reactor : public Basic {
public:
	Reactor() {
		mac = Random64::global_rand();
	}
	
	virtual void receive(Message* msg) override {
		msg->gate = this;
		if(!msg->dst) {
			msg->dst = this;
		}
		mutex.lock();
		if(msg->isack) {
			if(msg->callback) {
				(*msg->callback)(msg);
			}
			msg->release();
		} else {
			if(!handle(msg)) {
				msg->ack();
			}
		}
		mutex.unlock();
	}
	
protected:
	virtual bool handle(Message* msg) = 0;
	
	void send_async(Message* msg, Basic* dst) {
		msg->src = this;
		dst->receive(msg);
	}
	
private:
	std::mutex mutex;
	
};


class Actor : public Basic {
public:
	Actor() : ulock(mutex) {}
	
	virtual void receive(Message* msg) override {
		if(msg->isack) {
			std::unique_lock<std::mutex> lock(mutex);
			acked = true;
			cond.notify_all();
		}
	}
	
	void send(Message* msg, Basic* dst) {
		msg->src = this;
		acked = false;
		ulock.unlock();
		dst->receive(msg);
		ulock.lock();
		if(!acked) {
			cond.wait(ulock);
		}
	}
	
private:
	std::mutex mutex;
	std::condition_variable cond;
	std::unique_lock<std::mutex> ulock;
	
	bool acked = false;
	
};


}

#endif /* INCLUDE_PHY_NODE_H_ */
