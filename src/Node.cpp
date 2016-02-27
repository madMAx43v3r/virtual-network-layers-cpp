/*
 * Node.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include "Node.h"
#include "Uplink.h"
#include "Switch.h"

namespace vnl {

Node::Node(Uplink* uplink) : uplink(uplink)
{
	if(uplink) {
		phy::Object::send(Uplink::connect_t(this));
	}
}

Node::~Node() {
	
}

void Node::send(const Frame& frame, uint64_t sid, bool async) {
	if(uplink) {
		phy::Object::send(Uplink::send_t(frame, this, uplink, sid, async));
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
		uplink->receive(new Uplink::disconnect_t(this, 0, true));
	}
}


bool Switch::handle(phy::Message* msg) {
	phy::Object* obj = msg->src;
	if(!Uplink::handle(msg) && obj) {
		switch(msg->mid) {
		case connect_t::mid:
			nodes[obj->mac] = obj;
			msg->ack();
			return true;
		case disconnect_t::mid:
			nodes.erase(obj->mac);
			msg->ack();
			delete obj;
			return true;
		}
	}
	return false;
}


}
