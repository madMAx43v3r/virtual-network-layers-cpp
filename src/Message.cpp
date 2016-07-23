/*
 * Message.cpp
 *
 *  Created on: Feb 27, 2016
 *      Author: mad
 */

#include <vnl/Basic.h>
#include <vnl/Message.h>


namespace vnl {

void Message::ack() {
	assert(!isack);
	isack = true;
	if(src) {
		src->receive(this);
	}
}

vnl::String Message::to_string() {
	return vnl::String() << "[" << vnl::demangle(this) << "] mid=0x" << vnl::hex(msg_id)
			<< " src=" << src << " dst=" << dst << " isack=" << isack;
}


}
