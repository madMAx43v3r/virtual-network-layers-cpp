/*
 * Message.cpp
 *
 *  Created on: Feb 27, 2016
 *      Author: mad
 */

#include "phy/Message.h"
#include "phy/Object.h"

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

std::string Message::toString() {
	std::ostringstream ss;
	ss << "[" << Util::demangle(this) << "] mid=0x" << std::hex << mid << std::dec
			<< " src=" << src << " dst=" << dst << " isack=" << isack << " async=" << async;
	return ss.str();
}

}}
