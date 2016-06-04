/*
 * ByteInput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_BYTEINPUT_H_
#define INCLUDE_IO_BYTEINPUT_H_

#include "vnl/util/types.h"
#include "vnl/phy/Memory.h"


namespace vnl { namespace io {

template<typename TStream>
class ByteInput {
public:
	ByteInput(TStream& stream) : stream(stream) {}
	
	void get(void* buf, int32_t len) {
		err |= stream.read(buf, len);
	}
	
	void get(phy::Page* buf, int32_t len) {
		while(len > 0) {
			int32_t n = std::min(len, phy::Page::size);
			get(buf->mem, n);
			len -= n;
			if(len) {
				if(!buf->next) {
					buf->next = phy::Page::alloc();
				}
				buf = buf->next;
			}
		}
	}
	
	void getString(vnl::String& str, int32_t len) {
		while(len > 0) {
			char buf[1024];
			int32_t n = std::min(len, 1024);
			get(buf, n);
			str.write(buf, n);
			len -= n;
		}
	}
	
	void getChar(int8_t& value) {
		read(value);
	}
	
	void getShort(int16_t& value) {
		uint16_t tmp;
		read(tmp);
		value = vnl_ntohs(tmp);
	}
	
	void getInt(int32_t& value) {
		uint32_t tmp;
		read(tmp);
		value = vnl_ntohl(tmp);
	}
	
	void getLong(int64_t& value) {
		uint64_t tmp;
		read(tmp);
		value = vnl_ntohll(tmp);
	}
	
	void getFloat(float& value) {
		uint32_t tmp;
		read(tmp);
		value = vnl_ntohf(tmp);
	}
	
	void getDouble(double& value) {
		uint64_t tmp;
		read(tmp);
		value = vnl_ntohd(tmp);
	}
	
	bool error() {
		return err;
	}
	
protected:
	template<typename T>
	void read(T& data) {
		err |= stream->read(&data, sizeof(T));
	}
	
	TStream& stream;
	bool err = false;
	
};



}}

#endif /* INCLUDE_IO_BYTEINPUT_H_ */
