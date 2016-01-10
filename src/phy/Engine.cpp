/*
 * Engine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include "phy/Engine.h"

namespace vnl { namespace phy {

void Engine::forward(Message* msg) {
	msg->dst->receive(msg, msg->src);
}

Stream* Engine::get_stream(Object* obj, uint64_t sid) {
	return obj->get_stream(sid);
}

void Engine::dispatch(Message* msg) {
	msg->dst->handle(msg);
	if(!msg->isack) {
		msg->ack();
	}
}

void Engine::dispatch(Message* msg, Stream* stream) {
	stream->receive(msg, msg->dst);
}


}}
