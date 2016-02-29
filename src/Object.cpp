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

bool Object::handle(phy::Message* msg) {
	if(!Node::handle(msg) && msg->mid == receive_t::id) {
		const Frame& frame = ((receive_t*)msg)->frame;
		if(frame.flags & 0xF0 == 0) {
			receive(frame);
			return true;
		} else {
			switch(frame.flags) {
			case Frame::REGISTER:
				registered(frame.src, frame.dst);
				msg->ack();
				return true;
			case Frame::UNREGISTER:
				unregistered(frame.src, frame.dst);
				msg->ack();
				return true;
			}
		}
	}
	return false;
}


}
