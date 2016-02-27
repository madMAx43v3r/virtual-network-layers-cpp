/*
 * Node.h
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#ifndef INCLUDE_NODE_H_
#define INCLUDE_NODE_H_

#include "phy/Object.h"
#include "Frame.h"

namespace vnl {

class Switch;

class Node : public vnl::phy::Object {
public:
	Node(Uplink* uplink);
	~Node();
	
	typedef Packet<0xb494e3b0> receive_t;
	
protected:
	void send(const Frame& frame, uint64_t sid = 0, bool async = false);
	
	void configure(const Address& addr);
	void unregister(const Address& addr);
	
	void exit();
	
protected:
	Uplink* uplink;
	std::unordered_set<Address> logical;
	
};

}

#endif /* INCLUDE_NODE_H_ */
