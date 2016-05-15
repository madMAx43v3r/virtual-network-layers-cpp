/*
 * Reader.h
 *
 *  Created on: May 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_READER_H_
#define INCLUDE_PHY_READER_H_

#include <functional>

#include "Map.h"
#include "Address.h"
#include "phy/Packet.h"


namespace vnl { namespace phy {

template<typename T>
class Reader {
public:
	Reader(const vnl::Address& addr, const std::function<void(const T&)>& func) : addr(addr), func(func) {
		
	}
	
	bool handle(Packet* pkt) {
		if(pkt->dst_addr == addr) {
			
		}
		return false;
	}
	
private:
	vnl::Address addr;
	std::function<void(const T&)> func;
	
};


}}


#endif /* INCLUDE_PHY_READER_H_ */
