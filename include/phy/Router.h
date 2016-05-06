/*
 * Router.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ROUTER_H_
#define INCLUDE_PHY_ROUTER_H_

#include "phy/Node.h"
#include "phy/Map.h"
#include "phy/Packet.h"
#include "phy/Pool.h"


namespace vnl { namespace phy {

class Router : public Reactor {
public:
	static Router* instance;
	
	Router();
	
	typedef Generic<uint64_t, 0xdfd4dd65> connect_t;
	typedef Generic<uint64_t, 0x90bbb93d> close_t;
	
	typedef Generic<Packet*, 0xe46e436d> receive_t;
	typedef Generic<Packet*, 0xa5439117> send_t;
	
protected:
	virtual bool handle(Message* msg) override;
	
	int route(Packet* pkt, Node* src);
	
	void forward(Packet* pkt, Node* dst);
	
	void callback_rcv(Message* msg);
	
	Region mem;
	
private:
	typedef List<Node*> Row;
	
	Map<uint64_t, Row*> table;
	
	std::function<void(phy::Message*)> cb_rcv;
	Pool<receive_t> rcvbuf;
	
};


}}

#endif /* INCLUDE_PHY_ROUTER_H_ */
