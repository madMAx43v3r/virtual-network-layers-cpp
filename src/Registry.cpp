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
		Module* obj = connect(req->args);
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

bool Registry::bind(Module* obj) {
	//std::cout << "Registry::bind(" << std::hex << obj->mac << std::ios::dec << ")" << std::endl;
	uint64_t mac = obj->getMAC();
	Module*& value = map[mac];
	if(value == 0) {
		value = obj;
		obj->ref++;
		if(waiting.find(mac)) {
			auto& list = waiting[mac];
			for(connect_t* msg : list) {
				//std::cout << "Registry::bind(" << std::hex << obj->mac << "): ack for " << msg->src->getMAC() << std::ios::dec << std::endl;
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
	//std::cout << "Registry::connect(" << std::hex << mac << std::ios::dec << ")" << std::endl;
	Module** pobj = map.find(mac);
	if(pobj) {
		Module* obj = *pobj;
		open(obj);
		return obj;
	}
	return 0;
}

void Registry::open(Module* obj) {
	//std::cout << "Registry::open(" << std::hex << obj->mac << std::ios::dec << ")" << std::endl;
	obj->ref++;
}

void Registry::close(Module* obj) {
	//std::cout << "Registry::close(" << std::hex << obj->mac << std::ios::dec << ")" << std::endl;
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
	exit_t* msg = exit_buf.create();
	msg->callback = &cb_func;
	send_async(msg, obj);
}

void Registry::callback(Message* msg) {
	exit_buf.destroy((exit_t*)msg);
}



}
