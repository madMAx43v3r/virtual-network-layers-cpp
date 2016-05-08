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
	bool handle(Message* msg) override;
	
private:
	typedef Generic<Object*, 0x5a8a106d> finished_t;
	
	bool bind(Object* obj);
	
	Object* connect(uint64_t mac);
	
	void open(Object* obj);
	
	void close(Object* obj);
	
	void kill(Object* obj);
	
private:
	vnl::util::spinlock sync;
	std::unordered_map<uint64_t, Object*> map;
	std::unordered_map<uint64_t, std::vector<connect_t*> > waiting;
	
	Message* exit_msg = 0;
	
	friend class Engine;
	
};


}}

#endif /* INCLUDE_PHY_REGISTRY_H_ */
