/*
 * TypeOutput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_TYPEOUTPUT_H_
#define INCLUDE_IO_TYPEOUTPUT_H_

#include <vnl/io/ByteOutput.h>


namespace vnl { namespace io {

template<typename TStream>
class TypeOutput : public ByteOutput<TStream> {
public:
	static const int MAX_SIZE = 2147483647;
	
	typedef ByteOutput<TStream> Base;
	
	TypeOutput(TStream& stream) : ByteOutput<TStream>(stream) {}
	
	void putHash(uint32_t hash) {
		Base::writeInt(hash);
	}
	
	void putSize(int32_t size) {
		if(size > -128 && size < 128) {
			Base::writeChar(size);
		} else {
			Base::writeChar(-128);
			Base::writeInt(size);
		}
	}
	
	void putNull() {
		putSize(0);
	}
	
	void putChar(int8_t value) {
		putSize(1);
		Base::writeChar(value);
	}
	
	void putShort(int16_t value) {
		putSize(2);
		Base::writeShort(value);
	}
	
	void putInt(int32_t value) {
		putSize(4);
		Base::writeInt(value);
	}
	
	void putLong(int64_t value) {
		putSize(8);
		Base::writeLong(value);
	}
	
	void putFloat(float value) {
		putSize(4);
		Base::writeFloat(value);
	}
	
	void putDouble(double value) {
		putSize(8);
		Base::writeDouble(value);
	}
	
	void putBinary(Page* buf, int32_t len) {
		int32_t size = std::min(len, MAX_SIZE);
		putSize(size);
		Base::writeBinary(buf, size);
	}
	
	void putString(const vnl::String& str) {
		int32_t size = std::min(str.size(), MAX_SIZE);
		putSize(size);
		Base::writeString(str, size);
	}
	
};


}}

#endif /* INCLUDE_IO_TYPEOUTPUT_H_ */
