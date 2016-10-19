/*
 * Router.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ROUTER_H_
#define INCLUDE_PHY_ROUTER_H_

#include <vnl/Util.h>
#include <vnl/Pipe.h>
#include <vnl/Packet.h>
#include <vnl/Map.h>
#include <vnl/Array.h>
#include <vnl/Pool.h>


namespace vnl {

class Router : public Reactor {
public:
	static Router* instance;
	
	Router();
	
	typedef MessageType<vnl::pair<uint64_t, Address>, 0xbe3fa14f> open_t;
	typedef MessageType<vnl::pair<uint64_t, Address>, 0xfbe7dd5a> close_t;
	
	typedef MessageType<vnl::pair<Basic*, bool>, 0x1ea66ea3> hook_t;
	
	uint64_t num_drop = 0;
	uint64_t num_cycle = 0;
	
protected:
	typedef Array<uint64_t> Row;
	
	virtual bool handle(Message* msg) override;
	
	void open(const Address& addr, uint64_t src);
	void close(const Address& addr, uint64_t src);
	
	void route(Packet* packet, Basic* src, Row* prow);
	void forward(Packet* org, Basic* dst);
	
	virtual void callback(Message* msg);
	
	PageAllocator memory;
	MessagePool buffer;
	
private:
	Map<Address, Row> table;
	Map<uint64_t, Basic*> lookup;
	Basic* hook_dst;
	
};



}

#endif /* INCLUDE_PHY_ROUTER_H_ */
