/*
 * Buffer.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_BUFFER_H_
#define INCLUDE_IO_BUFFER_H_

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
			int left = Page::size - pos;
			if(!left) {
				left = fetch();
				if(left <= 0) {
					return;
				}
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
		return err;
	}
	
	void set_error(int err_) {
		err = err_;
#ifdef VNL_IO_DEBUG
		assert(err == VNL_IO_SUCCESS);
#endif
	}
	
protected:
	int fetch() {
		int n = in->read(buf->mem, Page::size);
		if(n <= 0) {
			set_error(VNL_IO_ERROR);
			return err;
		}
		pos = 0;
		return n;
	}
	
protected:
	InputStream* in = 0;
	Page* buf;
	int pos = 0;
	
private:
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
	
	void write(char c) {
		// TODO
	}
	
	void write(const void* src, int len) {
		while(len) {
			int left = Page::size - pos;
			if(!left) {
				if(!flush()) {
					set_error(VNL_IO_ERROR);
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
	
	int error() const {
		return err;
	}
	
	void set_error(int err_) {
		err = err_;
#ifdef VNL_IO_DEBUG
		assert(err == VNL_IO_SUCCESS);
#endif
	}
	
protected:
	OutputStream* out = 0;
	Page* buf;
	int pos = 0;
	
private:
	int err = 0;
	
};


}}

#endif /* INCLUDE_IO_BUFFER_H_ */
