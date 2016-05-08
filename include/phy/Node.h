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

#include "util/spinlock.h"
#include "phy/Random.h"


namespace vnl { namespace phy {

class Message;
class Engine;
class Fiber;
class Stream;


class Node {
public:
	virtual ~Node() {}
	
	uint64_t getMAC() const { return mac; }
	
	// must be thread safe !!!
	virtual void receive(Message* msg) = 0;
	
protected:
	uint64_t mac = 0;
	
private:
	Node* impl = 0;
	
	friend class Engine;
	friend class Stream;
	friend class Fiber;
	
};


class Reactor : public Node {
public:
	Reactor() {
		mac = Random64::global_rand();
	}
	
	virtual void receive(Message* msg) override {
		msg->gate = this;
		if(!msg->dst) {
			msg->dst = this;
		}
		sync.lock();
		if(msg->isack) {
			if(msg->callback) {
				msg->callback(msg);
			}
		} else {
			if(!handle(msg)) {
				msg->ack();
			}
		}
		sync.unlock();
	}
	
protected:
	virtual bool handle(Message* msg) = 0;
	
	void send_async(Message* msg, Node* dst) {
		msg->src = this;
		dst->receive(msg);
	}
	
private:
	vnl::util::spinlock sync;
	
};


class SyncNode : public Node {
public:
	SyncNode() : ulock(mutex) {}
	
	virtual void receive(Message* msg) override {
		std::unique_lock<std::mutex> lock(mutex);
		cond.notify_all();
	}
	
	void send(Message* msg, Node* dst) {
		msg->src = this;
		dst->receive(msg);
		cond.wait(ulock);
	}
	
private:
	std::mutex mutex;
	std::condition_variable cond;
	std::unique_lock<std::mutex> ulock;
	
};


}}

#endif /* INCLUDE_PHY_NODE_H_ */
