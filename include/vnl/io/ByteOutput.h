/*
 * ByteOutput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_BYTEOUTPUT_H_
#define INCLUDE_IO_BYTEOUTPUT_H_

#include <vnl/io/Stream.h>
#include <vnl/Util.h>
#include <vnl/String.h>


namespace vnl { namespace io {

class ByteOutput : public OutputBuffer {
public:
	ByteOutput(OutputStream* stream) : OutputBuffer(stream) {}
	
	void write(int8_t value) { writeChar(value); }
	void write(int16_t value) { writeShort(value); }
	void write(int32_t value) { writeInt(value); }
	void write(int64_t value) { writeLong(value); }
	void write(uint8_t value) { writeChar(value); }
	void write(uint16_t value) { writeShort(value); }
	void write(uint32_t value) { writeInt(value); }
	void write(uint64_t value) { writeLong(value); }
	void write(float value) { writeFloat(value); }
	void write(double value) { writeDouble(value); }
	
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
	
	void writeBinary(Page* buf, int len) {
		while(len > 0) {
			int n = std::min(len, Page::size);
			OutputBuffer::write(buf->mem, n);
			len -= n;
			buf = buf->next;
		}
	}
	
	void writeString(const vnl::String& str) {
		for(String::const_iterator it = str.begin(); it != str.end(); ++it) {
			write_type(*it);
		}
	}
	
private:
	template<typename T>
	void write_type(T data) {
		OutputBuffer::write(&data, sizeof(T));
	}
	
};



}}

#endif /* INCLUDE_IO_BYTEOUTPUT_H_ */
