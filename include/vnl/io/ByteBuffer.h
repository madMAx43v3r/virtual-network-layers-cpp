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
	ByteBuffer(Page* begin) : first(begin) {
		buf = first;
	}
	
	ByteBuffer(Page* begin, int limit) : ByteBuffer(begin) {
		lim = limit;
	}
	
	void reset() {
		buf = first;
		lim = 0;
		pos = 0;
		off = 0;
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
		if(!left) {
			if(!buf->next) {
				err = VNL_IO_UNDERFLOW;
				return 0;
			}
			buf = buf->next;
			off = 0;
			left = Page::size;
		}
		left = std::min(left, lim - pos);
		int n = std::min(len, left);
		memcpy(dst, buf->mem + off, n);
		pos += n;
		off += n;
		return n;
	}
	
	virtual bool write(const void* src, int len) {
		while(len) {
			int off = pos % Page::size;
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
		}
		return true;
	}
	
	int error() {
		return err;
	}
	
protected:
	Page* buf;
	Page* first;
	int lim = 0;
	int pos = 0;
	int off = 0;
	int err = 0;
	
};


}}

#endif /* INCLUDE_IO_PAGEBUFFER_H_ */
