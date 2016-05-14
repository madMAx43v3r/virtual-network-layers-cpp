/*
 * Registry.cpp
 *
 *  Created on: May 3, 2016
 *      Author: mad
 */

#include "phy/Registry.h"


namespace vnl { namespace phy {

Registry* Registry::instance = 0;

Registry::Registry()
	:	exit_buf(mem),
		cb_func(std::bind(&Registry::callback, this, std::placeholders::_1))
{
}

bool Registry::handle(Message* msg) {
	switch(msg->msg_id) {
	case bind_t::MID: {
		bind_t* req = (bind_t*)msg;
		req->ack(bind(req->args));
		return true;
	}
	case connect_t::MID: {
		connect_t* req = (connect_t*)msg;
		Object* obj = connect(req->args);
		if(obj) {
			req->ack(obj);
		} else {
			waiting[req->args].push_back(req);
		}
		return true;
	}
	case open_t::MID:
		open(((open_t*)msg)->data);
		msg->ack();
		return true;
	case close_t::MID:
		close(((close_t*)msg)->data);
		msg->ack();
		return true;
	case delete_t::MID:
		kill(((delete_t*)msg)->data);
		msg->ack();
		return true;
	case finished_t::MID:
		close(((finished_t*)msg)->data);
		msg->ack();
		return true;
	case shutdown_t::MID:
		if(exit_msg || map.empty()) {
			msg->ack();
		} else {
			for(auto pair : map.entries()) {
				kill(pair.second);
			}
			exit_msg = msg;
		}
		return true;
	}
	return false;
}

bool Registry::bind(Object* obj) {
	uint64_t mac = obj->getMAC();
	Object*& value = map[mac];
	if(value == 0) {
		value = obj;
		obj->ref++;
		if(waiting.find(mac)) {
			for(connect_t* msg : waiting[mac]) {
				obj->ref++;
				msg->ack(obj);
			}
			waiting.erase(mac);
		}
		return true;
	}
	return false;
}

Object* Registry::connect(uint64_t mac) {
	Object** pobj = map.find(mac);
	if(pobj) {
		Object* obj = *pobj;
		open(obj);
		return obj;
	}
	return 0;
}

void Registry::open(Object* obj) {
	obj->ref++;
}

void Registry::close(Object* obj) {
	obj->ref--;
	if(obj->dying) {
		if(obj->ref == 1) {
			send_exit(obj);
		} else if(obj->ref == 0) {
			map.erase(obj->getMAC());
			delete obj;
		}
	}
	if(exit_msg && map.empty()) {
		exit_msg->ack();
	}
}

void Registry::kill(Object* obj) {
	if(!obj->dying) {
		obj->dying = true;
		if(obj->ref == 1) {
			send_exit(obj);
		}
	}
}

void Registry::send_exit(Object* obj) {
	exit_t* msg = exit_buf.create();
	msg->callback = &cb_func;
	send_async(msg, obj);
}

void Registry::callback(Message* msg) {
	exit_buf.destroy((exit_t*)msg);
}



}}
