/*
 * Node.cpp
 *
 *  Created on: May 14, 2016
 *      Author: mad
 */

#include "Node.h"


namespace vnl {

bool Node::handle(phy::Message* msg) {
	if(msg->msg_id == Packet::MID) {
		vnl::Packet* pkt = (vnl::Packet*)msg;
		return handle(pkt);
	}
	return false;
}


}

