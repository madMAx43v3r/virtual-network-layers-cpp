/*
 * Registry.h
 *
 *  Created on: Apr 24, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_REGISTRY_H_
#define INCLUDE_PHY_REGISTRY_H_

#include "vnl/phy/Node.h"
#include "vnl/phy/Pool.h"
#include "vnl/Array.h"
#include "vnl/Map.h"


namespace vnl { namespace phy {

class Object;


class Registry : public vnl::phy::Reactor {
public:
	static Registry* instance;
	
	Registry();
	
	typedef RequestType<bool, Object*, 0x51d42b41> bind_t;
	typedef RequestType<Object*, uint64_t, 0x3127424a> connect_t;
	typedef MessageType<Object*, 0x88b4365a> open_t;
	typedef MessageType<Object*, 0x2120ef0e> close_t;
	typedef MessageType<Object*, 0x4177d786> delete_t;
	typedef SignalType<0x9a4ac2ca> exit_t;
	typedef SignalType<0x2aa87626> shutdown_t;
	
protected:
	bool handle(Message* msg) override;
	
private:
	typedef MessageType<Object*, 0x5a8a106d> finished_t;
	
	bool bind(Object* obj);
	
	Object* connect(uint64_t mac);
	
	void open(Object* obj);
	
	void close(Object* obj);
	
	void kill(Object* obj);
	
	void send_exit(Object* obj);
	
	void callback(Message* msg);
	
private:
	Region mem;
	Pool<exit_t> exit_buf;
	
	Map<uint64_t, Object*> map;
	Map<uint64_t, Array<connect_t*> > waiting;
	
	std::function<void(phy::Message*)> cb_func;
	Message* exit_msg = 0;
	
	friend class Engine;
	
};


}}

#endif /* INCLUDE_PHY_REGISTRY_H_ */
