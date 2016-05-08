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
	
	Packet(uint64_t dst) : dst(dst) {}
	
	Address src = 0;
	Address dst = 0;
	
private:
	Packet* parent = 0;
	int32_t count = 0;
	int32_t acks = 0;
	
	friend class Router;
	
};


template<typename T, uint64_t MAC>
class PacketType : public Packet {
public:
	PacketType() : Packet(id) {}
	
	template<typename R>
	PacketType(R&& data) : Packet(id), data(data) {}
	
	static const uint64_t id = MAC;
	
	T data;
	
	
};


}}

#endif /* INCLUDE_PHY_PACKET_H_ */
