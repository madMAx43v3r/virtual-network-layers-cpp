/*
 * Packet.h
 *
 *  Created on: Feb 26, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_PACKET_H_
#define INCLUDE_PHY_PACKET_H_

#include "Address.h"
#include "phy/Message.h"


namespace vnl { namespace phy {

class Packet : public Message {
public:
	static const uint32_t MID = 0xbd5fe6e6;
	
	Packet(uint32_t pid) : Message(MID), pid(pid) {}
	
	struct payload_t {
		RingBuffer* buffer = 0;
		RingBuffer::entry_t* entry = 0;
	};
	
	uint32_t pid;
	Address psrc;
	Address pdst;
	void* payload = 0;
	
	Packet* parent = 0;
	int32_t count = 0;
	int32_t acks = 0;
	
};


template<typename T, uint32_t PID_>
class PacketType : public Packet {
public:
	PacketType() : Packet(PID_) {
		payload = &data;
	}
	
	PacketType(T data_) : Packet(PID_), data(data_) {
		payload = &data;
	}
	
	PacketType(T data_, const Address& dst_) : Packet(PID_), data(data_) {
		payload = &data;
		pdst = dst_;
	}
	
	static const uint32_t PID = PID_;
	
	typedef T data_t;
	
	T data;
	
};


}}

#endif /* INCLUDE_PHY_PACKET_H_ */
