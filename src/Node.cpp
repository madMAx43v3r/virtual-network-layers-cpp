/*
 * Node.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include "Node.h"
#include "Uplink.h"

namespace vnl {

Node::Node(Uplink* uplink) : uplink(uplink)
{
	if(uplink) {
		phy::Object::send(Uplink::connect_t(), uplink);
	}
}

Node::~Node() {
	
}

void Node::send(const Frame& frame) {
	if(uplink) {
		phy::Object::send(send_t(frame, mac), uplink);
	}
}

void Node::configure(const Address& addr) {
	if(logical.insert(addr).second) {
		send(Frame(Frame::REGISTER, addr));
	}
}

void Node::unregister(const Address& addr) {
	if(logical.erase(addr)) {
		send(Frame(Frame::UNREGISTER, addr));
	}
}

void Node::exit() {
	for(auto addr : logical) {
		unregister(addr);
	}
	if(uplink) {
		phy::Object::send(Uplink::disconnect_t(), uplink);
		uplink = 0;
	}
}


}
