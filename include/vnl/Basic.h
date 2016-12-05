/*
 * Node.h
 *
 *  Created on: Apr 25, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_NODE_H_
#define INCLUDE_PHY_NODE_H_

#include <vnl/Random.h>
#include <vnl/Message.h>


namespace vnl {

class Basic {
public:
	Basic() {}
	
	virtual ~Basic() {}
	
	// must be thread safe !!!
	virtual void receive(Message* msg) {
		msg->ack();
	}
	
	Basic(const Basic&) = delete;
	Basic& operator=(const Basic&) = delete;
	
};


class Node : public Basic {
public:
	Node() : _impl(0) {
		mac = vnl::rand();
	}
	
	uint64_t get_mac() const { return mac; }
	
	void* _impl;
	
protected:
	uint64_t mac;
	
};


} // vnl

#endif /* INCLUDE_PHY_NODE_H_ */
