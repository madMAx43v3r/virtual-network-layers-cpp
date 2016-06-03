/*
 * TypeOutput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_TYPEOUTPUT_H_
#define INCLUDE_IO_TYPEOUTPUT_H_

#include "io/ByteOutput.h"


namespace vnl { namespace io {

template<typename TStream>
class TypeOutput {
public:
	static const int MAX_ENTRY_SIZE = 2147483642;
	
	TypeOutput(TStream& stream) : out(stream) {}
	
	void putEntry(uint32_t hash, int32_t size) {
		out.putInt(hash);
		if(size > -128 && size < 128) {
			out.putChar(size);
		} else {
			out.putChar(-128);
			out.putInt(size);
		}
	}
	
	void putNull() {
		putEntry(0, 0);
	}
	
	void putChar(uint32_t hash, int8_t value) {
		putEntry(hash, 1);
		out.putChar(value);
	}
	
	void putShort(uint32_t hash, int16_t value) {
		putEntry(hash, 2);
		out.putShort(value);
	}
	
	void putInt(uint32_t hash, int32_t value) {
		putEntry(hash, 4);
		out.putInt(value);
	}
	
	void putLong(uint32_t hash, int64_t value) {
		putEntry(hash, 8);
		out.putLong(value);
	}
	
	void putFloat(uint32_t hash, float value) {
		putEntry(hash, 4);
		out.putFloat(value);
	}
	
	void putDouble(uint32_t hash, double value) {
		putEntry(hash, 8);
		out.putDouble(value);
	}
	
	void putBinary(uint32_t hash, int32_t len, const void* data) {
		int size = std::min(len, MAX_ENTRY_SIZE);
		putEntry(hash, size);
		out.put(data, size);
	}
	
	void putString(uint32_t hash, const vnl::String& str) {
		int size = std::min(str.size(), MAX_ENTRY_SIZE);
		putEntry(hash, size);
		out.putString(str, size);
	}
	
	bool error() {
		return out.error();
	}
	
protected:
	io::ByteOutput<TStream>& out;
	
};


}}

#endif /* INCLUDE_IO_TYPEOUTPUT_H_ */
