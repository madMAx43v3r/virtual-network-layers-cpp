/*
 * TypeOutput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_TYPEOUTPUT_H_
#define INCLUDE_IO_TYPEOUTPUT_H_

#include "vnl/io/ByteOutput.h"


namespace vnl { namespace io {

template<typename TStream>
class TypeOutput : public ByteOutput<TStream> {
public:
	static const int MAX_SIZE = 2147483642;
	
	TypeOutput(TStream& stream) : ByteOutput<TStream>(stream) {}
	
	void putHash(uint32_t hash) {
		ByteOutput<TStream>::writeInt(hash);
	}
	
	void putSize(int32_t size) {
		if(size > -128 && size < 128) {
			ByteOutput<TStream>::writeChar(size);
		} else {
			ByteOutput<TStream>::writeChar(-128);
			ByteOutput<TStream>::writeInt(size);
		}
	}
	
	void putNull() {
		putSize(0);
	}
	
	void putChar(int8_t value) {
		putSize(1);
		ByteOutput<TStream>::writeChar(value);
	}
	
	void putShort(int16_t value) {
		putSize(2);
		ByteOutput<TStream>::writeShort(value);
	}
	
	void putInt(int32_t value) {
		putSize(4);
		ByteOutput<TStream>::writeInt(value);
	}
	
	void putLong(int64_t value) {
		putSize(8);
		ByteOutput<TStream>::writeLong(value);
	}
	
	void putFloat(float value) {
		putSize(4);
		ByteOutput<TStream>::writeFloat(value);
	}
	
	void putDouble(double value) {
		putSize(8);
		ByteOutput<TStream>::writeDouble(value);
	}
	
	void putBinary(phy::Page* buf, int32_t len) {
		int32_t size = std::min(len, MAX_SIZE);
		putSize(size);
		ByteOutput<TStream>::writeBinary(buf, size);
	}
	
	void putString(const vnl::String& str) {
		int32_t size = std::min(str.size(), MAX_SIZE);
		putSize(size);
		ByteOutput<TStream>::writeString(str, size);
	}
	
};


}}

#endif /* INCLUDE_IO_TYPEOUTPUT_H_ */
