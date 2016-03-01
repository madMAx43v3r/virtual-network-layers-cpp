/*
 * Node.h
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#ifndef INCLUDE_NODE_H_
#define INCLUDE_NODE_H_

#include <unordered_set>

#include "phy/Object.h"
#include "Packet.h"

namespace vnl {

class Uplink;

class Node : public vnl::phy::Object {
public:
	Node(Uplink* uplink);
	~Node();
	
	typedef PacketTmpl<0xb494e3b0> receive_t;
	typedef PacketTmpl<0xef34166d> send_t;
	
	std::unordered_set<Address> logical;
	
protected:
	void send(const Frame& frame, uint64_t sid = 0, bool async = false);
	
	void configure(const Address& addr);
	void unregister(const Address& addr);
	
	void exit();
	
protected:
	Uplink* uplink;
	
};

}

#endif /* INCLUDE_NODE_H_ */
