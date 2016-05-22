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
class TypeOutput : public io::ByteOutput<TStream> {
public:
	static const int MAX_ENTRY_SIZE = 32000;
	
	TypeOutput(TStream& stream) : ByteOutput<TStream>(stream), out(*this) {}
	
	void beginType(uint64_t hash, int32_t num_entries) {
		out.putLong(hash);
		out.putInt(num_entries);
	}
	
	void beginEntry(uint32_t hash, int16_t size) {
		out.putInt(hash);
		out.putShort(size);
	}
	
	void putType(uint32_t hash) {
		beginEntry(hash, -1);
	}
	
	void putChar(uint32_t hash, int8_t value) {
		beginEntry(hash, 1);
		out.putChar(value);
	}
	
	void putShort(uint32_t hash, int16_t value) {
		beginEntry(hash, 2);
		out.putShort(value);
	}
	
	void putInt(uint32_t hash, int32_t value) {
		beginEntry(hash, 4);
		out.putInt(value);
	}
	
	void putLong(uint32_t hash, int64_t value) {
		beginEntry(hash, 8);
		out.putLong(value);
	}
	
	void putFloat(uint32_t hash, float value) {
		beginEntry(hash, 4);
		out.putFloat(value);
	}
	
	void putDouble(uint32_t hash, double value) {
		beginEntry(hash, 8);
		out.putDouble(value);
	}
	
	void putBinary(uint32_t hash, int len, const void* data) {
		int size = std::min(len, MAX_ENTRY_SIZE);
		beginEntry(hash, size);
		out.put(data, size);
	}
	
	void putString(uint32_t hash, const vnl::String& str) {
		int size = std::min(str.size(), MAX_ENTRY_SIZE);
		beginEntry(hash, size);
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
