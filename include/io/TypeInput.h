/*
 * TypeInput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_TYPEINPUT_H_
#define INCLUDE_IO_TYPEINPUT_H_

#include "io/ByteInput.h"


namespace vnl { namespace io {

template<typename TStream>
class TypeInput : public io::ByteInput<TStream> {
public:
	TypeInput(TStream& stream) : ByteInput<TStream>(stream), in(*this) {}
	
	void beginType(uint64_t& hash, int32_t& num_entries) {
		in.getLong(hash);
		in.getInt(num_entries);
	}
	
	void beginEntry(uint32_t& hash, int16_t& size) {
		in.getInt(hash);
		in.getShort(size);
	}
	
	bool getType(uint32_t& hash) {
		int16_t size;
		beginEntry(hash, size);
		return size == -1;
	}
	
	void getChar(int8_t& value, int size) {
		switch(size) {
			case 1: { in.getChar(value); break; }
			case 2: { int16_t tmp; in.getShort(tmp); value = tmp; break; }
			case 4: { int32_t tmp; in.getInt(tmp); value = tmp; break; }
			case 8: { int64_t tmp; in.getLong(tmp); value = tmp; break; }
			default: err = true;
		}
	}
	
	void getShort(int16_t& value, int size) {
		switch(size) {
			case 1: { int8_t tmp; in.getChar(tmp); value = tmp; break; }
			case 2: { in.getShort(value); break; }
			case 4: { int32_t tmp; in.getInt(tmp); value = tmp; break; }
			case 8: { int64_t tmp; in.getLong(tmp); value = tmp; break; }
			default: err = true;
		}
	}
	
	void getInt(int32_t& value, int size) {
		switch(size) {
			case 1: { int8_t tmp; in.getChar(tmp); value = tmp; break; }
			case 2: { int16_t tmp; in.getShort(tmp); value = tmp; break; }
			case 4: { in.getInt(value); break; }
			case 8: { int64_t tmp; in.getLong(tmp); value = tmp; break; }
			default: err = true;
		}
	}
	
	void getLong(int64_t& value, int size) {
		switch(size) {
			case 1: { int8_t tmp; in.getChar(tmp); value = tmp; break; }
			case 2: { int16_t tmp; in.getShort(tmp); value = tmp; break; }
			case 4: { int32_t tmp; in.getInt(tmp); value = tmp; break; }
			case 8: { in.getLong(value); break; }
			default: err = true;
		}
	}
	
	void getFloat(float& value, int size) {
		switch(size) {
			case 4: { in.getFloat(value); break; }
			case 8: { double tmp; in.getDouble(tmp); value = tmp; break; }
			default: err = true;
		}
	}
	
	void getDouble(double& value, int size) {
		switch(size) {
			case 4: { float tmp; in.getFloat(tmp); value = tmp; break; }
			case 8: { in.getDouble(value); break; }
			default: err = true;
		}
	}
	
	void getBinary(void* data, int len, int max_len) {
		int n = std::min(len, max_len);
		in.get(data, n);
		len -= n;
		if(len) {
			skip(len);
		}
	}
	
	void getString(uint32_t& hash, vnl::String& str) {
		int16_t size;
		beginEntry(hash, size);
		in.getString(str, size);
	}
	
	void skip(int len) {
		char buf[1024];
		while(len > 0) {
			int n = std::min(len, 1024);
			in.get(buf, n);
			len -= n;
		}
	}
	
	bool error() {
		return err || in.error();
	}
	
protected:
	io::ByteInput<TStream>& in;
	bool err = false;
	
};


}}

#endif /* INCLUDE_IO_TYPEINPUT_H_ */
