/*
 * ByteOutput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_BYTEOUTPUT_H_
#define INCLUDE_IO_BYTEOUTPUT_H_

#include <vnl/util/types.h>
#include <vnl/io/Buffer.h>
#include <vnl/String.h>


namespace vnl { namespace io {

class ByteOutput : public OutputBuffer {
public:
	ByteOutput(OutputStream* stream) : OutputBuffer(stream) {}
	
	void writeBinary(Page* buf, int len) {
		while(len > 0) {
			int n = std::min(len, Page::size);
			write(buf->mem, n);
			len -= n;
			buf = buf->next;
		}
	}
	
	void writeString(const vnl::String& str) {
		int len = str.size();
		vnl::String::chunk_t* chunk = str.front();
		while(len > 0) {
			int n = std::min((int)chunk->len(), len);
			write(chunk->str(), n);
			len -= n;
			chunk = chunk->next_chunk();
		}
	}
	
	void writeChar(int8_t value) {
		write_type<int8_t>(value);
	}
	
	void writeShort(int16_t value) {
		write_type<int16_t>(vnl_htons(value));
	}
	
	void writeInt(int32_t value) {
		write_type<int32_t>(vnl_htonl(value));
	}
	
	void writeLong(int64_t value) {
		write_type<int64_t>(vnl_htonll(value));
	}
	
	void writeFloat(float value) {
		write_type<float>(vnl_htonf(value));
	}
	
	void writeDouble(double value) {
		write_type<double>(vnl_htond(value));
	}
	
private:
	template<typename T>
	void write_type(T data) {
		write(&data, sizeof(T));
	}
	
};



}}

#endif /* INCLUDE_IO_BYTEOUTPUT_H_ */
