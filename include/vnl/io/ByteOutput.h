/*
 * ByteOutput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_BYTEOUTPUT_H_
#define INCLUDE_IO_BYTEOUTPUT_H_

#include "vnl/util/types.h"
#include "vnl/phy/Memory.h"
#include "vnl/String.h"


namespace vnl { namespace io {

template<typename TStream>
class ByteOutput {
public:
	ByteOutput(TStream& stream) : stream(stream) {}
	
	void write(const void* buf, int32_t len) {
		stream.write(buf, len);
	}
	
	void writeBinary(phy::Page* buf, int32_t len) {
		while(len > 0) {
			int32_t n = std::min(len, phy::Page::size);
			write(buf->mem, n);
			len -= n;
			buf = buf->next;
		}
	}
	
	void writeString(const vnl::String& str, int32_t len) {
		vnl::String::chunk_t* chunk = str.front();
		while(chunk && chunk->len) {
			int32_t n = std::min((int32_t)chunk->len, len);
			write(chunk->str, n);
			len -= n;
			chunk = chunk->next;
		}
	}
	
	void writeChar(int8_t value) {
		write<int8_t>(value);
	}
	
	void writeShort(int16_t value) {
		write<int16_t>(vnl_htons(value));
	}
	
	void writeInt(int32_t value) {
		write<int32_t>(vnl_htonl(value));
	}
	
	void writeLong(int64_t value) {
		write<int64_t>(vnl_htonll(value));
	}
	
	void writeFloat(float value) {
		write<float>(vnl_htonf(value));
	}
	
	void writeDouble(double value) {
		write<double>(vnl_htond(value));
	}
	
protected:
	template<typename T>
	void write(T data) {
		stream.write(&data, sizeof(T));
	}
	
	TStream& stream;
	
};



}}


#endif /* INCLUDE_IO_BYTEOUTPUT_H_ */
