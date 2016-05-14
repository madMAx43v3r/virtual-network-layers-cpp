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

#define VNL_SAMPLE(type) typedef vnl::phy::SampleType<type> sample_t;


namespace vnl { namespace phy {

const uint32_t BIND = 0x2f45ca52;
const uint32_t CONNECT = 0xef835d78;
const uint32_t CLOSE = 0x9a9e3705;
const uint32_t SAMPLE = 0xe1167cfe;


class Packet : public Message {
public:
	static const uint32_t MID = 0xbd5fe6e6;
	
	Packet(uint32_t pid) : Message(MID), pkt_id(pid) {}
	
	struct payload_t {
		RingBuffer* buffer = 0;
		RingBuffer::entry_t* entry = 0;
	};
	
	uint32_t pkt_id;
	Address src_addr;
	Address dst_addr;
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
		dst_addr = dst_;
	}
	
	static const uint32_t PID = PID_;
	
	typedef T data_t;
	
	T data;
	
};


template<typename T>
class SampleType : public PacketType<T, SAMPLE> {
public:
	SampleType() : PacketType<T, SAMPLE>() {}
	
	SampleType(T data_) : PacketType<T, SAMPLE>(data_) {}
	
	SampleType(T data_, const Address& dst_) : PacketType<T, SAMPLE>(data_, dst_) {}
	
	
};


}}

#endif /* INCLUDE_PHY_PACKET_H_ */
