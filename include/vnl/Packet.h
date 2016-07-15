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
	uint32_t seq_num;
	Address src_addr;
	Address dst_addr;
	void* payload;
	
	int16_t num_hops;
	uint32_t route[VNL_MAX_ROUTE_LENGTH];
	
	Packet()
		:	Message(MID), pkt_id(0), seq_num(0), num_hops(0), payload(0) {}
	
	void copy_from(Packet* org) {
		parent = org;
		pkt_id = org->pkt_id;
		seq_num = org->seq_num;
		src_addr = org->src_addr;
		dst_addr = org->dst_addr;
		payload = org->payload;
		num_hops = org->num_hops;
		memcpy(route, org->route, sizeof(uint32_t)*VNL_MAX_ROUTE_LENGTH);
	}
	
	virtual void serialize(vnl::io::TypeOutput& out) const {
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_BEGIN);
		out.putHash(pkt_id);
		out.writeShort(4+16+16+2 + num_hops*4);
		out.writeInt(seq_num);
		src_addr.serialize(out);
		dst_addr.serialize(out);
		out.writeShort(num_hops);
		for(int i = 0; i < num_hops && !out.error(); ++i) {
			out.writeInt(route[i]);
		}
		write(out);
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_END);
	}
	
	virtual void deserialize(vnl::io::TypeInput& in, int size) {
		int16_t header = 0;
		in.readShort(header);
		in.readInt(seq_num);
		src_addr.deserialize(in, 0);
		dst_addr.deserialize(in, 0);
		in.readShort(num_hops);
		num_hops = std::min(num_hops, (int16_t)VNL_MAX_ROUTE_LENGTH);
		for(int i = 0; i < num_hops && !in.error(); ++i) {
			int32_t hash = 0;
			in.readInt(hash);
			route[i] = hash;
		}
		int left = header - (4+16+16+2 + num_hops*4);
		if(left > 0) {
			in.skip(VNL_IO_BINARY, left);
		}
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
	PacketType() : Packet() {
		pkt_id = PID_;
		payload = &data;
	}
	
	PacketType(const T& data_) : Packet(), data(data_) {
		pkt_id = PID_;
		payload = &data;
	}
	
	static const uint32_t PID = PID_;
	
	typedef T data_t;
	
	T data;
	
};



}

#endif /* INCLUDE_PHY_PACKET_H_ */
