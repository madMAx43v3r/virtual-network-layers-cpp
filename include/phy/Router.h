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
#include "phy/Pool.h"
#include "List.h"
#include "Map.h"
#include "Address.h"


namespace vnl { namespace phy {

class Router : public Reactor {
public:
	static Router* instance;
	
	Router();
	
	struct header_t {
		Message* parent = 0;
		int32_t count = 0;
		int32_t acks = 0;
		Packet* payload = 0;
	};
	
	typedef Generic<Address, 0xdfd4dd65> connect_t;
	typedef Generic<Address, 0x90bbb93d> close_t;
	typedef Generic<header_t, 0xe46e436d> packet_t;
	
	// thread safe
	template<typename T>
	void send(T&& node, Packet* packet, Address dst) {
		packet->dst = dst;
		header_t header;
		header.payload = packet;
		node.send(packet_t(header), this);
	}
	
	// thread safe
	template<typename T>
	void send_async(T&& node, Packet* packet, Address dst) {
		packet->dst = dst;
		header_t header;
		header.payload = packet;
		node.send_async(packet_t(header), this);
	}
	
protected:
	typedef List<Node*> Row;
	
	virtual bool handle(Message* msg) override;
	
	void connect(const Address& addr, Node* src);
	void close(const Address& addr, Node* src);
	
	void route(packet_t* packet, Node* src, Row** prow);
	void forward(packet_t* packet, Node* dst);
	
	void callback(Message* msg);
	
	Region mem;
	
private:
	Map<Address, Row*> table;
	
	std::function<void(phy::Message*)> cb_func;
	Pool<packet_t> factory;
	
};


}}

#endif /* INCLUDE_PHY_ROUTER_H_ */
