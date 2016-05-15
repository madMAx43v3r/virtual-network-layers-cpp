/*
 * Router.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ROUTER_H_
#define INCLUDE_PHY_ROUTER_H_

#include "phy/Node.h"
#include "phy/RingBuffer.h"
#include "Packet.h"
#include "Map.h"
#include "List.h"
#include "Address.h"


namespace vnl {

class Router : public phy::Reactor {
public:
	static Router* instance;
	
	Router();
	
	typedef phy::MessageType<Address, 0xbe3fa14f> open_t;
	typedef phy::MessageType<Address, 0xfbe7dd5a> close_t;
	
protected:
	typedef List<phy::Node*> Row;
	
	virtual bool handle(phy::Message* msg) override;
	
	void open(const Address& addr, phy::Node* src);
	void close(const Address& addr, phy::Node* src);
	
	void route(Packet* packet, phy::Node* src, Row** prow);
	void forward(Packet* org, phy::Node* dst);
	
	void callback(phy::Message* msg);
	
	phy::Region mem;
	phy::MessageBuffer buffer;
	
private:
	Map<Address, Row*> table;
	std::function<void(phy::Message*)> cb_func;
	
};



}

#endif /* INCLUDE_PHY_ROUTER_H_ */
