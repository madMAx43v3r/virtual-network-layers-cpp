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

class Uplink;

class Node : public vnl::phy::Object {
public:
	Node(Uplink* uplink);
	~Node();
	
	typedef Packet<0xb494e3b0> receive_t;
	
protected:
	void send(const Frame& frame);
	
	void configure(const Address& addr);
	void unregister(const Address& addr);
	
protected:
	phy::Object* uplink;
	std::unordered_set<Address> logical;
	
};

}

#endif /* INCLUDE_NODE_H_ */
