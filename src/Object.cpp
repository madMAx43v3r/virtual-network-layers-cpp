/*
 * Object.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include "Object.h"

namespace vnl {

Object::Object(Uplink* uplink) : Node(uplink) {
	phy::Object::send(Switch::connect_t(uplink));
}

Object::~Object() {
	for(auto addr : logical) {
		unregister(addr);
	}
	phy::Object::send(Switch::disconnect_t(uplink));
}

void Object::handle(phy::Message* msg) {
	Node::handle(msg);
	if(msg->mid == receive_t::mid) {
		const Frame& frame = ((receive_t*)msg)->frame;
		if(frame.flags & 0xF0 == 0) {
			receive(frame);
			msg->ack();
		} else {
			switch(frame.flags) {
			case Frame::REGISTER:
				registered(frame.src, frame.dst);
				msg->ack();
				break;
			case Frame::UNREGISTER:
				unregistered(frame.src, frame.dst);
				msg->ack();
				break;
			}
		}
	}
	
}


}
