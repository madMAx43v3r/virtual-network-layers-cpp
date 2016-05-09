/*
 * Packet.h
 *
 *  Created on: Feb 26, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_PACKET_H_
#define INCLUDE_PHY_PACKET_H_

#include "Address.h"


namespace vnl { namespace phy {

class Router;

class Packet {
public:
	Packet() {}
	
	Address src;
	Address dst;
	
};


template<typename T>
class PacketType : public Packet {
public:
	PacketType() : Packet() {}
	
	template<typename R>
	PacketType(R&& data_) : Packet(), data(data_) {}
	
	T data;
	
	
};


}}

#endif /* INCLUDE_PHY_PACKET_H_ */
