/*
 * Frame.h
 *
 *  Created on: 16.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_FRAME_H_
#define INCLUDE_FRAME_H_

#include "Address.h"
#include "Struct.h"
#include "ByteBuffer.h"

namespace vnl {

class Frame {
public:
	static const char NONE = 0x00;
	static const char UNICAST = 0x01;
	static const char ANYCAST = 0x02;
	static const char MULTICAST = 0x04;
	
	static const char REGISTER = 0x10 | MULTICAST;
	static const char UNREGISTER = 0x20 | MULTICAST;
	
	char flags;
	Address src;
	Address dst;
	Struct* data;
	
	Frame() {
		flags = NONE;
		data = 0;
	}
	
	Frame(char flags, const Address& dst, Struct* data = 0) {
		this->flags = flags;
		this->dst = dst;
		this->data = data;
	}
	
	bool isNull() {
		return flags == NONE;
	}
	
	bool serialize(vnl::io::Stream* stream) {
		ByteBuffer buf(stream);
		buf.put(flags);
		buf.putLong(src.A);
		buf.putLong(src.B);
		buf.putLong(dst.A);
		buf.putLong(dst.B);
		if(data) {
			buf.putLong(data->vnl_id);
			buf.error |= !data->serialize(stream);
		} else {
			buf.putLong(0);
		}
		return !buf.error;
	}
	
	bool deserialize(vnl::io::Stream* stream) {
		ByteBuffer buf(stream);
		flags = buf.get();
		src.A = buf.getLong();
		src.B = buf.getLong();
		dst.A = buf.getLong();
		dst.B = buf.getLong();
		uint64_t id = buf.getLong();
		if(id) {
			Struct::type_t type = Struct::resolve(id);
			if(type.id) {
				data = type.create();
				data->deserialize(stream);
			}
		}
		return !buf.error;
	}
	
};


}

#endif /* INCLUDE_FRAME_H_ */
