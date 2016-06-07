/*
 * Router.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ROUTER_H_
#define INCLUDE_PHY_ROUTER_H_

#include "vnl/Base.h"
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
	
	typedef MessageType<Address, 0xbe3fa14f> open_t;
	typedef MessageType<Address, 0xfbe7dd5a> close_t;
	
protected:
	typedef List<Base*> Row;
	
	virtual bool handle(Message* msg) override;
	
	void open(const Address& addr, Base* src);
	void close(const Address& addr, Base* src);
	
	void route(Packet* packet, Base* src, Row** prow);
	void forward(Packet* org, Base* dst);
	
	void callback(Message* msg);
	
	Region mem;
	MessageBuffer buffer;
	
private:
	Map<Address, Row*> table;
	std::function<void(Message*)> cb_func;
	
};



}

#endif /* INCLUDE_PHY_ROUTER_H_ */
