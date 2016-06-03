/*
 * PageBuffer.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_PAGEBUFFER_H_
#define INCLUDE_IO_PAGEBUFFER_H_

#include <string.h>
#include "phy/Memory.h"


namespace vnl { namespace io {

class PageBuffer {
public:
	PageBuffer(phy::Page* data) : first(data) {
		buf = first;
	}
	
	void reset() {
		buf = first;
		limit = 0;
		pos = 0;
		off = 0;
	}
	
	void flip() {
		buf = first;
		limit = pos;
		pos = 0;
		off = 0;
	}
	
	int position() const {
		return pos;
	}
	
	bool read(void* dst, int len) {
		while(len) {
			int left = phy::Page::size - off;
			if(!left) {
				if(!buf->next) {
					return false;
				}
				buf = buf->next;
				off = 0;
				left = phy::Page::size;
			}
			int n = std::min(len, left);
			memcpy(dst, buf->mem + off, n);
			off += n;
			len -= n;
			dst = (char*)dst + n;
		}
		pos += len;
		if(pos <= limit) {
			return true;
		} else {
			return false;
		}
	}
	
	bool write(const void* src, int len) {
		while(len) {
			int left = phy::Page::size - off;
			if(!left) {
				if(!buf->next) {
					buf->next = phy::Page::alloc();
				}
				buf = buf->next;
				off = 0;
				left = phy::Page::size;
			}
			int n = std::min(len, left);
			memcpy(buf->mem + off, src, n);
			off += n;
			len -= n;
			src = (const char*)src + n;
		}
		pos += len;
		return true;
	}
	
protected:
	phy::Page* buf;
	phy::Page* first;
	int limit = 0;
	int pos = 0;
	int off = 0;
	
};


}}

#endif /* INCLUDE_IO_PAGEBUFFER_H_ */