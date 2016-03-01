/*
 * Switch.cpp
 *
 *  Created on: Feb 26, 2016
 *      Author: mad
 */

#include "Switch.h"

namespace vnl {

bool Switch::handle(phy::Message* msg) {
	if(Uplink::handle(msg)) {
		return msg;
	}
	phy::Object* obj = msg->src;
	if(obj) {
		switch(msg->mid) {
		case connect_t::id:
			nodes[obj->mac] = obj;
			msg->ack();
			return true;
		case disconnect_t::id:
			nodes.erase(obj->mac);
			msg->ack();
			return true;
		}
	}
	return false;
}


}
