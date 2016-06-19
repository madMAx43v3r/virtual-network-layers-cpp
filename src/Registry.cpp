/*
 * Registry.cpp
 *
 *  Created on: May 3, 2016
 *      Author: mad
 */

#include "vnl/Registry.h"
#include "vnl/Module.h"


namespace vnl {

Registry* Registry::instance = 0;

Registry::Registry()
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
		Module* obj = connect(req->args);
		if(obj) {
			req->ack(obj);
		} else {
			waiting[req->args].push_back(req);
		}
		return true;
	}
	case try_connect_t::MID: {
		try_connect_t* req = (try_connect_t*)msg;
		req->ack(connect(req->args));
		return true;
	}
	case open_t::MID:
		open(((open_t*)msg)->data);
		break;
	case close_t::MID:
		close(((close_t*)msg)->data);
		break;
	case delete_t::MID:
		kill(((delete_t*)msg)->data);
		break;
	case finished_t::MID:
		close(((finished_t*)msg)->data);
		break;
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
	case get_module_list_t::MID:
		((get_module_list_t*)msg)->data = map.keys();
		break;
	}
	return false;
}

bool Registry::bind(Module* obj) {
	uint64_t mac = obj->getMAC();
	Module*& value = map[mac];
	if(value == 0) {
		value = obj;
		obj->ref++;
		if(waiting.find(mac)) {
			auto& list = waiting[mac];
			for(connect_t* msg : list) {
				obj->ref++;
				msg->ack(obj);
			}
			list.clear();
			waiting.erase(mac);
		}
		return true;
	}
	return false;
}

Module* Registry::connect(uint64_t mac) {
	Module** pobj = map.find(mac);
	if(pobj) {
		Module* obj = *pobj;
		open(obj);
		return obj;
	}
	return 0;
}

void Registry::open(Module* obj) {
	obj->ref++;
}

void Registry::close(Module* obj) {
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

void Registry::kill(Module* obj) {
	if(!obj->dying) {
		obj->dying = true;
		if(obj->ref == 1) {
			send_exit(obj);
		}
	}
}

void Registry::send_exit(Module* obj) {
	exit_t* msg = buffer.create<exit_t>();
	send_async(msg, obj);
}



}
