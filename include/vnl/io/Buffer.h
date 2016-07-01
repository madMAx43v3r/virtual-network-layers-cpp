/*
 * Buffer.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_BUFFER_H_
#define INCLUDE_IO_BUFFER_H_

#include <string.h>
#include <vnl/Memory.h>
#include <vnl/io/Error.h>
#include <vnl/io/Stream.h>


namespace vnl { namespace io {

class InputBuffer {
public:
	InputBuffer(InputStream* in) : in(in) {
		buf = vnl::Page::alloc();
	}
	
	~InputBuffer() {
		buf->free();
	}
	
	void read(void* dst, int len) {
		while(len) {
			int left = Page::size - pos;
			if(!left) {
				left = in->read(buf->mem, Page::size);
				if(left <= 0) {
					err = VNL_IO_ERROR;
					return;
				}
				pos = 0;
			}
			int n = std::min(len, left);
			memcpy(dst, buf->mem + pos, n);
			dst = (char*)dst + n;
			len -= n;
			pos += n;
		}
		return;
	}
	
	bool error() {
		return err;
	}
	
protected:
	InputStream* in = 0;
	Page* buf;
	int pos = 0;
	int err = 0;
	
};


class OutputBuffer {
public:
	OutputBuffer(OutputStream* out) : out(out) {
		buf = vnl::Page::alloc();
	}
	
	~OutputBuffer() {
		buf->free();
	}
	
	void write(const void* src, int len) {
		while(len) {
			int left = Page::size - pos;
			if(!left) {
				if(!flush()) {
					err = VNL_IO_ERROR;
					return;
				}
			}
			int n = std::min(len, left);
			memcpy(buf->mem + pos, src, n);
			src = (const char*)src + n;
			len -= n;
			pos += n;
		}
		return;
	}
	
	bool flush() {
		bool res = out->write(buf->mem, pos);
		pos = 0;
		return res;
	}
	
	bool error() {
		return err;
	}
	
protected:
	OutputStream* out = 0;
	Page* buf;
	int pos = 0;
	int err = 0;
	
};


}}

#endif /* INCLUDE_IO_BUFFER_H_ */
