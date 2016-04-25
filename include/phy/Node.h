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


namespace vnl { namespace phy {

class Message;

class Node {
public:
	virtual ~Node() {}
	
	virtual void receive(Message* msg) = 0;
	
};


class SyncNode : public Node {
public:
	SyncNode() : ulock(mutex) {}
	
	virtual void receive(Message* msg) override {
		std::unique_lock<std::mutex> lock(mutex);
		cond.notify_all();
	}
	
	void wait() {
		cond.wait(ulock);
	}
	
private:
	std::mutex mutex;
	std::condition_variable cond;
	std::unique_lock<std::mutex> ulock;
	
};


}}

#endif /* INCLUDE_PHY_NODE_H_ */
