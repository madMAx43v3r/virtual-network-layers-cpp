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

bool Object::handle(phy::Message* msg) {
	if(msg->mid == receive_t::id) {
		const Frame& frame = ((receive_t*)msg)->frame;
		switch(frame.flags & 0xF0) {
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
	return false;
}


}
