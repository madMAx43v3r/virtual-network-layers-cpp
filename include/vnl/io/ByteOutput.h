/*
 * ByteOutput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_BYTEOUTPUT_H_
#define INCLUDE_IO_BYTEOUTPUT_H_

#include "util/types.h"
#include "phy/Memory.h"
#include "String.h"


namespace vnl { namespace io {

template<typename TStream>
class ByteOutput {
public:
	ByteOutput(TStream& stream) : stream(stream) {}
	
	void put(const void* buf, int32_t len) {
		err |= stream.write(buf, len);
	}
	
	void put(phy::Page* buf, int32_t len) {
		while(len > 0) {
			int32_t n = std::min(len, phy::Page::size);
			put(buf->mem, n);
			len -= n;
			buf = buf->next;
		}
	}
	
	void putString(const vnl::String& str, int32_t len) {
		vnl::String::chunk_t* chunk = str.front();
		while(chunk && chunk->len) {
			int32_t n = std::min((int32_t)chunk->len, len);
			put(chunk->str, n);
			len -= n;
			chunk = chunk->next;
		}
	}
	
	void putChar(int8_t value) {
		write<int8_t>(value);
	}
	
	void putShort(int16_t value) {
		write<int16_t>(vnl_htons(value));
	}
	
	void putInt(int32_t value) {
		write<int32_t>(vnl_htonl(value));
	}
	
	void putLong(int64_t value) {
		write<int64_t>(vnl_htonll(value));
	}
	
	void putFloat(float value) {
		write<float>(vnl_htonf(value));
	}
	
	void putDouble(double value) {
		write<double>(vnl_htond(value));
	}
	
	bool error() {
		return err;
	}
	
protected:
	template<typename T>
	bool write(T data) {
		err |= stream.write(&data, sizeof(T));
	}
	
	TStream& stream;
	bool err = false;
	
};



}}


#endif /* INCLUDE_IO_BYTEOUTPUT_H_ */
