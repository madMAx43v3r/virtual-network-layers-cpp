/*
 * Message.cpp
 *
 *  Created on: Feb 27, 2016
 *      Author: mad
 */

#include "vnl/phy/Message.h"
#include "vnl/phy/Object.h"

namespace vnl { namespace phy {

void Message::ack() {
	if(!isack) {
		isack = true;
		if(src) {
			src->receive(this);
		} else {
			delete this;
		}
	}
}

std::string Message::to_string() {
	std::ostringstream ss;
	ss << "[" << vnl::demangle(this) << "] mid=0x" << std::hex << msg_id << std::dec
			<< " src=" << src << " dst=" << dst << " isack=" << isack;
	return ss.str();
}

}}
