/*
 * Packet.h
 *
 *  Created on: Feb 26, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_PACKET_H_
#define INCLUDE_PHY_PACKET_H_

#include "vnl/Address.h"
#include "vnl/Message.h"

#define VNL_SAMPLE(type) typedef vnl::PacketType<type, vnl::SAMPLE> sample_t;


namespace vnl {

static const uint32_t SAMPLE = 0x12ed1215;

class Router;

class Packet : public Message {
public:
	static const uint32_t MID = 0xbd5fe6e6;
	
	Packet(uint32_t pid) : Message(MID), pkt_id(pid) {}
	
	uint32_t pkt_id;
	Address src_addr;
	Address dst_addr;
	void* payload = 0;
	
private:
	Packet* parent = 0;
	int32_t count = 0;
	int32_t acks = 0;
	
	friend class Router;
	
};


template<typename T, uint32_t PID_>
class PacketType : public Packet {
public:
	PacketType() : Packet(PID_) {
		payload = &data;
	}
	
	PacketType(const T& data_) : Packet(PID_), data(data_) {
		payload = &data;
	}
	
	PacketType(const T& data_, const Address& dst_) : Packet(PID_), data(data_) {
		payload = &data;
		dst_addr = dst_;
	}
	
	static const uint32_t PID = PID_;
	
	typedef T data_t;
	
	T data;
	
};



}

#endif /* INCLUDE_PHY_PACKET_H_ */
