/*
 * Registry.h
 *
 *  Created on: Apr 24, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_REGISTRY_H_
#define INCLUDE_PHY_REGISTRY_H_

#include <vnl/Basic.h>
#include "vnl/RingBuffer.h"
#include "vnl/Array.h"
#include "vnl/Map.h"


namespace vnl {

class Module;


class Registry : public Reactor {
public:
	static Registry* instance;
	
	Registry();
	
	typedef RequestType<bool, Module*, 0x51d42b41> bind_t;
	typedef RequestType<Module*, uint64_t, 0x3127424a> connect_t;
	typedef MessageType<Module*, 0x88b4365a> open_t;
	typedef MessageType<Module*, 0x2120ef0e> close_t;
	typedef MessageType<Module*, 0x4177d786> delete_t;
	typedef SignalType<0x9a4ac2ca> exit_t;
	typedef SignalType<0x2aa87626> shutdown_t;
	
protected:
	bool handle(Message* msg) override;
	
private:
	typedef MessageType<Module*, 0x5a8a106d> finished_t;
	
	bool bind(Module* obj);
	
	Module* connect(uint64_t mac);
	
	void open(Module* obj);
	
	void close(Module* obj);
	
	void kill(Module* obj);
	
	void send_exit(Module* obj);
	
private:
	PageAlloc memory;
	MessageBuffer buffer;
	
	Map<uint64_t, Module*> map;
	Map<uint64_t, Array<connect_t*> > waiting;
	
	std::function<void(Message*)> cb_func;
	Message* exit_msg = 0;
	
	friend class Engine;
	
};


}

#endif /* INCLUDE_PHY_REGISTRY_H_ */
