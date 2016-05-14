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
	
	class bind_t : public Packet {
	public:
		bind_t(const Address& addr) : Packet(BIND) { dst_addr = addr; }
	};
	
	class connect_t : public Packet {
	public:
		connect_t(const Address& addr) : Packet(CONNECT) { dst_addr = addr; }
	};
	
	class close_t : public Packet {
	public:
		close_t(const Address& addr) : Packet(CLOSE) { dst_addr = addr; }
	};
	
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
