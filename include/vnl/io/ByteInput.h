/*
 * ByteInput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_BYTEINPUT_H_
#define INCLUDE_IO_BYTEINPUT_H_

#include <vnl/io/Stream.h>
#include <vnl/Util.h>
#include <vnl/String.h>


namespace vnl { namespace io {

class ByteInput : public InputBuffer {
public:
	ByteInput(InputStream* stream) : InputBuffer(stream) {}
	
	void readChar(int8_t& value) {
		read_type(value);
	}
	void readChar(uint8_t& value) {
		read_type(value);
	}
	
	void readShort(int16_t& value) {
		uint16_t tmp;
		read_type(tmp);
		value = vnl_ntohs(tmp);
	}
	void readShort(uint16_t& value) {
		uint16_t tmp;
		read_type(tmp);
		value = vnl_ntohs(tmp);
	}
	
	void readInt(int32_t& value) {
		uint32_t tmp;
		read_type(tmp);
		value = vnl_ntohl(tmp);
	}
	void readInt(uint32_t& value) {
		uint32_t tmp;
		read_type(tmp);
		value = vnl_ntohl(tmp);
	}
	
	void readLong(int64_t& value) {
		uint64_t tmp;
		read_type(tmp);
		value = vnl_ntohll(tmp);
	}
	void readLong(uint64_t& value) {
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
	
	void readBinary(Page*& first, int len) {
		if(!first) {
			first = vnl::Page::alloc();
		}
		Page* buf = first;
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
		str.clear();
		while(len > 0) {
			char buf[1024];
			int n = std::min(len, 1024);
			read(buf, n);
			str.write(buf, n);
			len -= n;
		}
	}
	
private:
	template<typename T>
	void read_type(T& data) {
		read(&data, sizeof(T));
	}
	
};



}}

#endif /* INCLUDE_IO_BYTEINPUT_H_ */
