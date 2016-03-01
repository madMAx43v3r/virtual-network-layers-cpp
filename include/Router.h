/*
 * Router.h
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#ifndef INCLUDE_ROUTER_H_
#define INCLUDE_ROUTER_H_

#include <vector>
#include <unordered_map>

#include "Switch.h"

namespace vnl {

class Router : public Uplink {
public:
	Router(Uplink* uplink = 0);
	~Router();
	
protected:
	virtual bool handle(phy::Message* msg) override;
	
	void route(Packet* msg, uint64_t srcmac);
	void forward(Packet* msg, Node* dst);
	void fw_one(Packet* msg, std::vector<Node*>& list, bool anycast);
	void fw_many(Packet* msg, std::vector<Node*>& list, uint64_t srcmac);
	
	void configure(Address addr, Node* src);
	void unregister(Address addr, Node* src);
	
	std::vector<Node*>& get_entry(const Address& addr);
	void clear_entry(const Address& addr);
	Node* get_node(uint64_t mac);
	
private:
	std::unordered_map<uint64_t, Node*> nodes;
	std::unordered_map<uint64_t, std::vector<Node*> > route64;
	std::unordered_map<Address, std::vector<Node*> > route128;
	
	
	
};


}

#endif /* INCLUDE_ROUTER_H_ */
