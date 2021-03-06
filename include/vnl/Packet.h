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


namespace vnl {

class Router;
class Client;

class Packet : public Message, public io::Serializable {
public:
	static const uint32_t MID = 0xbd5fe6e6;
	
	enum {
		NO_DROP = 1,
	};
	
	uint32_t pkt_id;
	int64_t seq_num;
	Address src_addr;
	Address dst_addr;
	Packet* payload;
	
	int16_t num_hops;
	uint32_t route[VNL_MAX_ROUTE_LENGTH];
	uint64_t proxy;
	
	Packet()
		:	Message(MID), pkt_id(0), seq_num(0), num_hops(0), payload(0), proxy(0)
	{
		memset(route, 0, sizeof(route));
	}
	
	void copy_from(Packet* org) {
		parent = org;
		pkt_id = org->pkt_id;
		seq_num = org->seq_num;
		src_mac = org->src_mac;
		src_addr = org->src_addr;
		dst_addr = org->dst_addr;
		payload = org->payload;
		num_hops = org->num_hops;
		memcpy(route, org->route, sizeof(uint32_t)*org->num_hops);
		proxy = org->proxy;
		timeout = org->timeout;
		is_no_drop = org->is_no_drop;
	}
	
	int16_t get_header_size() const {
		return 8+8+16+16+2 + num_hops*4 + 1;
	}
	
	virtual void serialize(vnl::io::TypeOutput& out) const {
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_BEGIN);
		out.putHash(pkt_id);
		out.writeShort(get_header_size());
		out.writeLong(seq_num);
		out.writeLong(src_mac);
		out.writeLong(src_addr.A);
		out.writeLong(src_addr.B);
		out.writeLong(dst_addr.A);
		out.writeLong(dst_addr.B);
		out.writeShort(num_hops);
		for(int i = 0; i < num_hops && !out.error(); ++i) {
			out.writeInt(route[i]);
		}
		out.writeChar(is_no_drop ? NO_DROP : 0);
		assert(payload);
		payload->write(out);
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_END);
	}
	
	virtual void deserialize(vnl::io::TypeInput& in, int size) {
		int16_t header_len = 0;
		in.readShort(header_len);
		in.readLong(seq_num);
		in.readLong(src_mac);
		in.readLong(src_addr.A);
		in.readLong(src_addr.B);
		in.readLong(dst_addr.A);
		in.readLong(dst_addr.B);
		in.readShort(num_hops);
		num_hops = std::min(num_hops, (int16_t)VNL_MAX_ROUTE_LENGTH);
		for(int i = 0; i < num_hops && !in.error(); ++i) {
			int32_t hash = 0;
			in.readInt(hash);
			route[i] = hash;
		}
		uint8_t flags = 0;
		if(header_len >= get_header_size()) {
			in.readChar(flags);
		}
		is_no_drop = flags & NO_DROP;
		int left = header_len - get_header_size();
		if(left > 0) {
			in.skip(VNL_IO_BINARY, left);
		}
		assert(payload);
		payload->read(in);
		while(!in.error()) {
			int id = in.getEntry(size);
			if(id == VNL_IO_INTERFACE && size == VNL_IO_END) {
				break;
			}
			in.skip(id, size);
		}
	}
	
protected:
	virtual void write(vnl::io::TypeOutput& out) const {}
	virtual void read(vnl::io::TypeInput& in) {}
	
private:
	Packet* parent = 0;
	int count = 0;
	int acks = 0;
	
	friend class Router;
	friend class Client;
	friend class Message;
	
};


}

#endif /* INCLUDE_PHY_PACKET_H_ */
