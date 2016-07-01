/*
 * ByteInput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_BYTEINPUT_H_
#define INCLUDE_IO_BYTEINPUT_H_

#include <vnl/util/types.h>
#include <vnl/io/Buffer.h>
#include <vnl/String.h>


namespace vnl { namespace io {

class ByteInput : public InputBuffer {
public:
	ByteInput(InputStream* stream) : InputBuffer(stream) {}
	
	void readBinary(Page* buf, int len) {
		while(len > 0) {
			int32_t n = std::min(len, Page::size);
			read(buf->mem, n);
			len -= n;
			if(len) {
				if(!buf->next) {
					buf->next = Page::alloc();
				}
				buf = buf->next;
			}
		}
	}
	
	void readString(vnl::String& str, int len) {
		while(len > 0) {
			char buf[1024];
			int n = std::min(len, 1024);
			read(buf, n);
			str.write(buf, n);
			len -= n;
		}
	}
	
	void readChar(int8_t& value) {
		read_type(value);
	}
	
	void readShort(int16_t& value) {
		uint16_t tmp;
		read_type(tmp);
		value = vnl_ntohs(tmp);
	}
	
	void readInt(int32_t& value) {
		uint32_t tmp;
		read_type(tmp);
		value = vnl_ntohl(tmp);
	}
	
	void readLong(int64_t& value) {
		uint64_t tmp;
		read_type(tmp);
		value = vnl_ntohll(tmp);
	}
	
	void readFloat(float& value) {
		uint32_t tmp;
		read_type(tmp);
		value = vnl_ntohf(tmp);
	}
	
	void readDouble(double& value) {
		uint64_t tmp;
		read_type(tmp);
		value = vnl_ntohd(tmp);
	}
	
private:
	template<typename T>
	void read_type(T& data) {
		read(&data, sizeof(T));
	}
	
};



}}

#endif /* INCLUDE_IO_BYTEINPUT_H_ */
