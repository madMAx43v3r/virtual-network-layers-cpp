/*
 * Router.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ROUTER_H_
#define INCLUDE_PHY_ROUTER_H_

#include <vnl/Util.h>
#include <vnl/Reactor.h>
#include <vnl/Packet.h>
#include <vnl/Map.h>
#include <vnl/Array.h>
#include <vnl/Pool.h>

#include <vnl/info/TopicInfo.hxx>


namespace vnl {

class Router : public Reactor {
public:
	static Router* instance;
	
	Router();
	
	typedef MessageType<Node*, 0x9567b3a8> connect_t;
	typedef MessageType<Node*, 0xc22ce373 > finish_t;
	
	typedef MessageType<vnl::pair<uint64_t, Address>, 0xbe3fa14f> open_t;
	typedef MessageType<vnl::pair<uint64_t, Address>, 0xfbe7dd5a> close_t;
	
	typedef MessageType<vnl::Array<vnl::info::TopicInfo>, 0x477a9769> get_topic_info_t;
	
	bool enable_topic_info = true;
	
	uint64_t num_drop = 0;
	uint64_t num_cycle = 0;
	
protected:
	typedef Array<uint64_t> Row;
	
	virtual bool handle(Message* msg);
	
	void open(const Address& addr, uint64_t src);
	void close(const Address& addr, uint64_t src);
	
	void route(Packet* packet, Row* prow);
	void forward(Packet* org, Basic* dst);
	
	virtual void callback(Message* msg);
	
	MessagePool<Packet> buffer;
	
private:
	Map<Address, Row> table;
	Map<uint64_t, Basic*> lookup;
	Row* spy_list;
	
	vnl::info::TopicInfo* current_info;
	Map<Address, vnl::info::TopicInfo> topic_info;
	
};



}

#endif /* INCLUDE_PHY_ROUTER_H_ */
