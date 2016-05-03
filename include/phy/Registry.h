/*
 * Registry.h
 *
 *  Created on: Apr 24, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_REGISTRY_H_
#define INCLUDE_PHY_REGISTRY_H_

#include <unordered_map>

#include "phy/Object.h"


namespace vnl { namespace phy {

class Registry : public vnl::phy::Reactor {
public:
	static Registry* instance;
	
	typedef Request<bool, Object*, 0x51d42b41> bind_t;
	typedef Request<Object*, uint64_t, 0x3127424a> connect_t;
	typedef Generic<Object*, 0x88b4365a> open_t;
	typedef Generic<Object*, 0x2120ef0e> close_t;
	typedef Generic<Object*, 0x4177d786> delete_t;
	typedef Signal<0x2aa87626> shutdown_t;
	
protected:
	bool handle(Message* msg) override {
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
	
private:
	typedef Generic<Object*, 0x5a8a106d> finished_t;
	
	bool bind(Object* obj) {
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
	
	Object* connect(uint64_t mac) {
		auto iter = map.find(mac);
		if(iter != map.end()) {
			Object* obj = iter->second;
			open(obj);
			return obj;
		}
		return 0;
	}
	
	void open(Object* obj) {
		obj->ref++;
	}
	
	void close(Object* obj) {
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
	
	void kill(Object* obj) {
		if(!obj->dying) {
			obj->dying = true;
			if(obj->ref == 1) {
				obj->receive(new Object::exit_t());
			}
			obj->ref--;
		}
	}
	
private:
	vnl::util::spinlock sync;
	std::unordered_map<uint64_t, Object*> map;
	std::unordered_map<uint64_t, std::vector<connect_t*> > waiting;
	
	Message* exit_msg = 0;
	
	friend class Engine;
	
};


}}

#endif /* INCLUDE_PHY_REGISTRY_H_ */
