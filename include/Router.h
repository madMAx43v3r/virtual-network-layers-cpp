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

#include "Uplink.h"

namespace vnl {

class Router : public Uplink {
public:
	Router(Uplink* uplink = 0);
	
protected:
	void handle(phy::Message* msg) override;
	
	void route(Frame& frame, uint64_t srcmac);
	void forward(Frame& frame, phy::Object* dst);
	void fw_one(Frame& frame, std::vector<phy::Object*>& list, bool anycast);
	void fw_many(Frame& frame, std::vector<phy::Object*>& list, uint64_t srcmac);
	
	void configure(Address addr, phy::Object* src);
	void unregister(Address addr, phy::Object* src);
	
	std::vector<phy::Object*>& get_entry(const Address& addr);
	void clear_entry(const Address& addr);
	
private:
	std::unordered_map<uint64_t, std::vector<phy::Object*> > route64;
	std::unordered_map<Address, std::vector<phy::Object*> > route128;
	
};


}

#endif /* INCLUDE_ROUTER_H_ */
