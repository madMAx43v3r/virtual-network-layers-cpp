/*
 * Router.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ROUTER_H_
#define INCLUDE_PHY_ROUTER_H_

#include "phy/Node.h"
#include "phy/Packet.h"
#include "phy/RingBuffer.h"
#include "List.h"
#include "Map.h"
#include "Address.h"


namespace vnl { namespace phy {

class Router : public Reactor {
public:
	static Router* instance;
	
	Router();
	
	typedef MessageType<Address, 0xdfd4dd65> connect_t;
	typedef MessageType<Address, 0x90bbb93d> close_t;
	
protected:
	typedef List<Node*> Row;
	
	virtual bool handle(Message* msg) override;
	
	void connect(const Address& addr, Node* src);
	void close(const Address& addr, Node* src);
	
	void route(Packet* packet, Node* src, Row** prow);
	void forward(Packet* org, Node* dst);
	
	void callback(Message* msg);
	
	Region mem;
	MessageBuffer buffer;
	
private:
	Map<Address, Row*> table;
	std::function<void(phy::Message*)> cb_func;
	
};


}}

#endif /* INCLUDE_PHY_ROUTER_H_ */
