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

template<uint32_t MID>
class Packet : public vnl::phy::Message {
public:
	Packet() {}
	
	Packet(const Frame& frame, uint64_t sid = 0, bool async = false)
		:	Message(id, sid, async), frame(frame)
	{
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
	
	// TODO
	int32_t seq = 0;
	
	Frame frame;
	
};


}

#endif /* INCLUDE_PACKET_H_ */
