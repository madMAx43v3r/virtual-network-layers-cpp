/*
 * Packet.h
 *
 *  Created on: Feb 26, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_PACKET_H_
#define INCLUDE_PHY_PACKET_H_

#include "phy/Message.h"


namespace vnl { namespace phy {


class Packet {
public:
	Packet() {}
	
	Packet(uint64_t dst) : dst(dst) {}
	
	uint64_t src = 0;
	uint64_t dst = 0;
	
	Packet* parent = 0;
	int32_t count = 0;
	int32_t acks = 0;
	
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
