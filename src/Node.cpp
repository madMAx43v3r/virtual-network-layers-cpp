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

void Node::send(const Frame& frame, uint64_t sid, bool async) {
	if(uplink) {
		phy::Object::send(Uplink::send_t(frame, this, uplink, sid, async));
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
	switch(msg->mid) {
	case connect_t::mid:
		nodes[msg->src->mac] = msg->src;
		msg->ack();
		break;
	case disconnect_t::mid:
		nodes.erase(msg->src->mac);
		msg->ack();
		break;
	}
}


}
