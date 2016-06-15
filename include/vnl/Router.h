/*
 * Router.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ROUTER_H_
#define INCLUDE_PHY_ROUTER_H_

#include <vnl/Basic.h>
#include "vnl/RingBuffer.h"
#include "vnl/Packet.h"
#include "vnl/Map.h"
#include "vnl/List.h"
#include "vnl/Address.h"


namespace vnl {

class Router : public Reactor {
public:
	static Router* instance;
	
	Router();
	
	typedef MessageType<std::pair<Basic*, Address>, 0xbe3fa14f> open_t;
	typedef MessageType<std::pair<Basic*, Address>, 0xfbe7dd5a> close_t;
	
protected:
	typedef List<Basic*> Row;
	
	virtual bool handle(Message* msg) override;
	
	void open(const Address& addr, Basic* src);
	void close(const Address& addr, Basic* src);
	
	void route(Packet* packet, Basic* src, Row** prow);
	void forward(Packet* org, Basic* dst);
	
	void callback(Message* msg);
	
	Region mem;
	MessageBuffer buffer;
	
private:
	Map<Address, Row*> table;
	std::function<void(Message*)> cb_func;
	
};



}

#endif /* INCLUDE_PHY_ROUTER_H_ */
