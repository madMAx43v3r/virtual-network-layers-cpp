/*
 * Packet.h
 *
 *  Created on: Feb 26, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_PACKET_H_
#define INCLUDE_PHY_PACKET_H_

#include <vnl/Address.h>
#include <vnl/Message.h>
#include <vnl/io.h>

#define VNL_SAMPLE(type) typedef vnl::PacketType<type, vnl::PID_SAMPLE> sample_t;


namespace vnl {

static const uint32_t PID_SAMPLE = 0x12ed1215;

class Router;

class Packet : public Message, public io::Serializable {
public:
	static const uint32_t MID = 0xbd5fe6e6;
	
	uint32_t pkt_id;
	Address src_addr;
	Address dst_addr;
	void* payload = 0;
	
	Packet(uint32_t pid) : Message(MID), pkt_id(pid) {}
	
	virtual void serialize(vnl::io::TypeOutput& out) const {
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_BEGIN);
		out.putHash(pkt_id);
		src_addr.serialize(out);
		dst_addr.serialize(out);
		write(out);
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_END);
	}
	
	virtual void deserialize(vnl::io::TypeInput& in, int size) {
		src_addr.deserialize(in, 0);
		dst_addr.deserialize(in, 0);
		while(!in.error()) {
			int id = in.getEntry(size);
			if(id == VNL_IO_INTERFACE && size == VNL_IO_END) {
				break;
			}
			read(in, id, size);
		}
	}
	
protected:
	virtual void write(vnl::io::TypeOutput& out) const {
		out.putNull();
	}
	
	virtual void read(vnl::io::TypeInput& in, int id, int size) {
		in.skip(id, size);
	}
	
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
	
	static const uint32_t PID = PID_;
	
	typedef T data_t;
	
	T data;
	
};



}

#endif /* INCLUDE_PHY_PACKET_H_ */
