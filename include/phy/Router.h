/*
 * Router.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ROUTER_H_
#define INCLUDE_PHY_ROUTER_H_

#include "phy/Object.h"
#include "phy/List.h"
#include "phy/Map.h"


namespace vnl { namespace phy {

class Router : public Reactor {
public:
	Router(Object* uplink) : uplink(uplink) {
		
	}
	
	typedef Generic<uint32_t, 0xdfd4dd65> connect_t;
	typedef Generic<uint32_t, 0x90bbb93d> close_t;
	typedef Generic<uint32_t, 0xa5439117> send_t;
	typedef Generic<uint32_t, 0xe46e436d> receive_t;
	
protected:
	virtual bool handle(Message* msg) override {
		
	}
	
	Region mem;
	
private:
	Object* uplink;
	Map<uint32_t, List<Object*>* > route;
	
};


}}

#endif /* INCLUDE_PHY_ROUTER_H_ */
