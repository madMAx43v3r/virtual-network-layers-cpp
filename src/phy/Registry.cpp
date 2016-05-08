/*
 * Registry.cpp
 *
 *  Created on: May 3, 2016
 *      Author: mad
 */

#include "phy/Registry.h"


namespace vnl { namespace phy {

Registry* Registry::instance = 0;

bool Registry::handle(Message* msg) {
	switch(msg->mid) {
	case bind_t::id: {
		bind_t* req = (bind_t*)msg;
		req->ack(bind(req->args));
		return true;
	}
	case connect_t::id: {
		connect_t* req = (connect_t*)msg;
		Object* obj = connect(req->args);
		if(obj) {
			req->ack(obj);
		} else {
			waiting[req->args].push_back(req);
		}
		return true;
	}
	case open_t::id:
		open(((open_t*)msg)->data);
		msg->ack();
		return true;
	case close_t::id:
		close(((close_t*)msg)->data);
		msg->ack();
		return true;
	case delete_t::id:
		kill(((delete_t*)msg)->data);
		msg->ack();
		return true;
	case finished_t::id:
		close(((finished_t*)msg)->data);
		msg->ack();
		return true;
	case shutdown_t::id:
		if(!exit_msg) {
			for(auto pair : map) {
				kill(pair.second);
			}
			exit_msg = msg;
		} else {
			msg->ack();
		}
		return true;
	}
	if(exit_msg && map.empty()) {
		exit_msg->ack();
	}
	return false;
}

bool Registry::bind(Object* obj) {
	uint64_t mac = obj->getMAC();
	Object*& value = map[mac];
	if(value == 0) {
		value = obj;
		obj->ref++;
		if(waiting.count(mac)) {
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
	auto iter = map.find(mac);
	if(iter != map.end()) {
		Object* obj = iter->second;
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
			obj->receive(new Object::exit_t());
		} else if(obj->ref == 0) {
			map.erase(obj->getMAC());
			delete obj;
		}
	}
}

void Registry::kill(Object* obj) {
	if(!obj->dying) {
		obj->dying = true;
		if(obj->ref == 1) {
			obj->receive(new Object::exit_t());
		}
		obj->ref--;
	}
}



}}
