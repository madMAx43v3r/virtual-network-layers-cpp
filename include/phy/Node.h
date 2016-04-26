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


namespace vnl { namespace phy {

class Message;

class Node {
public:
	virtual ~Node() {}
	
	// must be thread safe !!!
	virtual void receive(Message* msg) = 0;
	
};


class FloatingNode : public Node {
public:
	
	virtual void receive(Message* msg) override {
		sync.lock();
		if(!handle(msg)) {
			msg->ack();
		}
		sync.unlock();
	}
	
protected:
	virtual bool handle(Message* msg) = 0;
	
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
