/*
 * Object.cpp
 *
 *  Created on: Mar 7, 2016
 *      Author: mad
 */

#include "phy/Object.h"

namespace vnl { namespace phy {

void Object::mainloop() {
	Message* shutdown = 0;
	while(true) {
		Message* msg = poll();
		if(!msg) {
			break;
		}
		switch(msg->mid) {
		case Engine::finished_t::id:
			tasks.erase(((Engine::finished_t*)msg)->data);
			msg->ack();
			break;
		case close_t::id:
			shutdown = msg;
			if(handle(msg)) {
				return;
			}
			break;
		default:
			if(!handle(msg)) {
				msg->ack();
			}
		}
		if(shutdown && tasks.size() == 0) {
			shutdown->ack();
			return;
		}
	}
}

}}
