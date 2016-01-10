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
}

Node::~Node() {
	for(auto addr : logical) {
		unregister(addr);
	}
}

void Node::send(const Frame& frame) {
	if(uplink) {
		phy::Object::send(Uplink::send_t(frame, this, uplink));
	}
}

void Node::configure(const Address& addr) {
	send(Frame(Frame::REGISTER, addr));
	logical.insert(addr);
}

void Node::unregister(const Address& addr) {
	send(Frame(Frame::UNREGISTER, addr));
	logical.erase(addr);
}


void Uplink::handle(phy::Message* msg) {
	if(!msg->src) {
		return;
	}
	uint64_t srcmac = msg->src->mac;
	switch(msg->mid) {
	case Uplink::connect_t::mid:
		nodes[srcmac] = msg->src;
		msg->ack();
		break;
	case Uplink::disconnect_t::mid:
		nodes.erase(srcmac);
		msg->ack();
		break;
	}
}

}
