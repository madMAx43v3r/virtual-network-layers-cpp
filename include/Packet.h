/*
 * Packet.h
 *
 *  Created on: Feb 26, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PACKET_H_
#define INCLUDE_PACKET_H_

#include "phy/Message.h"
#include "phy/Object.h"
#include "Frame.h"

namespace vnl {

class Packet : public vnl::phy::Message {
public:
	Packet* parent = 0;
	int32_t count = 0;
	int32_t acks = 0;
	
	int32_t seq = 0;
	Frame frame;
	
};

template<uint32_t MID>
class PacketTmpl : public Packet {
public:
	PacketTmpl() {}
	
	PacketTmpl(const Frame& frame_, uint64_t sid_ = 0, bool async_ = false) {
		mid = id;
		sid = sid_;
		async = async_;
		frame = frame_;
	}
	
	bool serialize(vnl::io::Stream* stream) {
		ByteBuffer out(stream);
		out.putInt(seq);
		out.putLong(sid);
		out.error |= !frame.serialize(stream);
		return !out.error;
	}
	
	bool deserialize(vnl::io::Stream* stream) {
		ByteBuffer in(stream);
		seq = in.getInt();
		sid = in.getLong();
		in.error |= !frame.deserialize(stream);
		return !in.error && Message::mid == id;
	}
	
	static const uint32_t id = MID;
	
	
};


}

#endif /* INCLUDE_PACKET_H_ */
