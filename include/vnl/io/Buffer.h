/*
 * Buffer.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_BUFFER_H_
#define INCLUDE_IO_BUFFER_H_

#include <string.h>
#include "phy/Memory.h"


namespace vnl { namespace io {

class Buffer {
public:
	Buffer(phy::Page* data) : buf(data) {}
	
	void reset() {
		limit = 0;
		pos = 0;
	}
	
	void flip() {
		limit = pos;
		pos = 0;
	}
	
	int position() const {
		return pos;
	}
	
	bool read(void* dst, int len) {
		if(phy::Page::size - pos < len) {
			return false;
		}
		memcpy(dst, buf->mem + pos, len);
		pos += len;
		return true;
	}
	
	bool write(const void* src, int len) {
		if(phy::Page::size - pos < len) {
			return false;
		}
		memcpy(buf->mem + pos, src, len);
		pos += len;
		return true;
	}
	
protected:
	phy::Page* buf;
	int limit = 0;
	int pos = 0;
	
};


}}

#endif /* INCLUDE_IO_BUFFER_H_ */
