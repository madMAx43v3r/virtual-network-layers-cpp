/*
 * Frame.h
 *
 *  Created on: 16.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_FRAME_H_
#define INCLUDE_FRAME_H_

#include <phy/Memory.h>
#include <vector>

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
	
	Address src;
	Address dst;
	phy::Page* data;
	int32_t size;
	
	char flags;
	
	Frame() {
		flags = NONE;
		data = 0;
		size = 0;
	}
	
	Frame(char flags, const Address& dst) {
		this->flags = flags;
		this->dst = dst;
		this->size = 0;
		this->data = 0;
	}
	
	Frame(char flags, const Address& dst, phy::Page* data, int32_t size) {
		this->flags = flags;
		this->dst = dst;
		this->size = size;
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
		buf.putInt(size);
		if(data) {
			buf.put(data, size);
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
		size = buf.getInt();
		if(size) {
			data = phy::Page::alloc();
			buf.get(data, size);
		}
		return !buf.error;
	}
	
};


}

#endif /* INCLUDE_FRAME_H_ */
