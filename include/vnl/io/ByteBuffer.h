/*
 * PageBuffer.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_PAGEBUFFER_H_
#define INCLUDE_IO_PAGEBUFFER_H_

#include <string.h>
#include <vnl/Memory.h>
#include <vnl/io/Stream.h>


namespace vnl { namespace io {

class ByteBuffer : public InputStream, public OutputStream {
public:
	ByteBuffer() : ByteBuffer(0) {}
	
	ByteBuffer(Page* begin) {
		wrap(begin);
	}
	
	ByteBuffer(Page* begin, int size) {
		wrap(begin, size);
	}
	
	void wrap(Page* begin) {
		first = begin;
		reset();
	}
	
	void wrap(Page* begin, int size) {
		wrap(begin);
		lim = size;
	}
	
	Page* release() {
		Page* tmp = first;
		first = 0;
		reset();
		return tmp;
	}
	
	void reset() {
		buf = first;
		lim = 0;
		pos = 0;
		off = 0;
		InputStream::err = 0;
		OutputStream::err = 0;
	}
	
	void flip() {
		buf = first;
		lim = pos;
		pos = 0;
		off = 0;
	}
	
	int position() const {
		return pos;
	}
	
	int limit() const {
		return lim;
	}
	
	virtual int read(void* dst, int len) {
		int left = Page::size - off;
		int max_left = lim - pos;
		if(max_left < left) {
			if(max_left <= 0) {
				InputStream::set_error(VNL_IO_UNDERFLOW);
				return InputStream::err;
			}
			left = max_left;
		}
		if(!left) {
			buf = buf->next;
			off = 0;
			left = std::min(Page::size, max_left);
		}
		int n = std::min(len, left);
		memcpy(dst, buf->mem + off, n);
		pos += n;
		off += n;
		return n;
	}
	
	virtual bool write(const void* src, int len) {
		if(!buf) {
			wrap(Page::alloc());
		}
		while(len) {
			int left = Page::size - off;
			if(!left) {
				if(!buf->next) {
					buf->next = Page::alloc();
				}
				buf = buf->next;
				off = 0;
				left = Page::size;
			}
			int n = std::min(len, left);
			memcpy(buf->mem + off, src, n);
			src = (const char*)src + n;
			len -= n;
			pos += n;
			off += n;
		}
		return true;
	}
	
protected:
	Page* buf;
	Page* first;
	int lim = 0;
	int pos = 0;
	int off = 0;
	
};


}}

#endif /* INCLUDE_IO_PAGEBUFFER_H_ */
