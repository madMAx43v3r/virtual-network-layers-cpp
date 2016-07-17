/*
 * Buffer.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_STREAMBUFFER_H_
#define INCLUDE_IO_STREAMBUFFER_H_

#include <string.h>
#include <algorithm>
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
	
	char read() {
		int left = Page::size - pos;
		if(!left) {
			left = fetch();
			if(left <= 0) {
				return 0;
			}
		}
		return buf->mem[pos++];
	}
	
	void read(void* dst, int len) {
		while(len) {
			int left = limit - pos;
			if(!left) {
				limit = fetch();
				if(limit <= 0) {
					return;
				}
				left = limit;
			}
			int n = std::min(len, left);
			memcpy(dst, buf->mem + pos, n);
			dst = (char*)dst + n;
			len -= n;
			pos += n;
		}
		return;
	}
	
	int error() const {
		return in->error();
	}
	
	void set_error(int err_) {
		in->set_error(err_);
	}
	
protected:
	int fetch() {
		pos = 0;
		int n = in->read(buf->mem, Page::size);
		if(n <= 0) {
			return n;
		}
		return n;
	}
	
protected:
	InputStream* in = 0;
	Page* buf;
	int pos = 0;
	int limit = 0;
	
};


class OutputBuffer {
public:
	OutputBuffer(OutputStream* out) : out(out) {
		buf = vnl::Page::alloc();
	}
	
	~OutputBuffer() {
		buf->free();
	}
	
	void write(char c) {
		// TODO
		assert(false);
	}
	
	void write(const void* src, int len) {
		while(len) {
			int left = Page::size - pos;
			if(!left) {
				if(!flush()) {
					return;
				}
				left = Page::size;
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
		if(pos) {
			if(!out->write(buf->mem, pos)) {
				return false;
			}
			pos = 0;
		}
		return true;
	}
	
	int error() const {
		return out->error();
	}
	
	void set_error(int err_) {
		out->set_error(err_);
	}
	
protected:
	OutputStream* out = 0;
	Page* buf;
	int pos = 0;
	
};


}}

#endif /* INCLUDE_IO_STREAMBUFFER_H_ */
