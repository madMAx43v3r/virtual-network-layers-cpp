/*
 * Object.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include "Object.h"

namespace vnl {

Object::Object(Uplink* uplink) : Node::Node(uplink) {
	
}

Object::~Object() {
	
}

void Object::handle(phy::Message* msg) {
	if(!Node::handle(msg) && msg->mid == receive_t::mid) {
		const Frame& frame = ((receive_t*)msg)->frame;
		if(frame.flags & 0xF0 == 0) {
			receive(frame);
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
