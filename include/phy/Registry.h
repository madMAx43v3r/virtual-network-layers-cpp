/*
 * Registry.h
 *
 *  Created on: Apr 24, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_REGISTRY_H_
#define INCLUDE_PHY_REGISTRY_H_

#include <unordered_map>

#include "phy/Node.h"


namespace vnl { namespace phy {

class Registry : public vnl::phy::FloatingNode {
public:
	static Registry* instance;
	
	typedef vnl::phy::Request<bool, Object*, 0x51d42b41> bind_t;
	typedef vnl::phy::Request<Object*, uint64_t, 0x3127424a> connect_t;
	typedef vnl::phy::Generic<uint64_t, 0x2120ef0e> close_t;
	typedef vnl::phy::Generic<Object*, 0x4177d786> kill_t;
	typedef vnl::phy::Signal<0x2aa87626> shutdown_t;
	
protected:
	struct entry_t {
		Object* obj = 0;
		int64_t ref = 0;
		bool dying = false;
	};
	
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
		case close_t::id:
			close(((close_t*)msg)->data);
			msg->ack();
			return true;
		case kill_t::id:
			kill(((kill_t*)msg)->data);
			msg->ack();
			return true;
		case Engine::finished_t::id:
			finished(((Engine::finished_t*)msg)->data);
			msg->ack();
			return true;
		case shutdown_t::id:
			if(!exit_msg) {
				for(auto pair : map) {
					kill(pair.second.obj);
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
	bool bind(Object* obj) {
		uint64_t mac = obj->MAC();
		entry_t& entry = map[mac];
		if(entry.obj == 0) {
			entry.obj = obj;
			entry.ref++;
			if(waiting.count(mac)) {
				for(connect_t* msg : waiting[mac]) {
					entry.ref++;
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
			entry_t& entry = iter->second;
			entry.ref++;
			return entry.obj;
		}
		return 0;
	}
	
	void close(uint64_t mac) {
		auto iter = map.find(mac);
		if(iter != map.end()) {
			entry_t& entry = iter->second;
			entry.ref--;
			if(entry.dying) {
				if(entry.ref == 1) {
					entry.obj->receive(new Object::exit_t());
				} else if(entry.ref == 0) {
					delete entry.obj;
					map.erase(iter);
				}
			}
		}
	}
	
	void kill(Object* obj) {
		uint64_t mac = obj->MAC();
		auto iter = map.find(mac);
		if(iter != map.end()) {
			entry_t& entry = iter->second;
			if(!entry.dying) {
				entry.dying = true;
				dying[obj->task.id] = mac;
				if(entry.ref == 1) {
					entry.obj->receive(new Object::exit_t());
				}
			}
		}
	}
	
	void finished(uint64_t taskid) {
		auto iter = dying.find(taskid);
		if(iter != dying.end()) {
			close(iter->second);
			dying.erase(iter);
		}
	}
	
private:
	vnl::util::spinlock sync;
	std::unordered_map<uint64_t, entry_t> map;
	std::unordered_map<uint64_t, std::vector<connect_t*> > waiting;
	std::unordered_map<uint64_t, uint64_t> dying;
	
	Message* exit_msg = 0;
	
};


}}

#endif /* INCLUDE_PHY_REGISTRY_H_ */
