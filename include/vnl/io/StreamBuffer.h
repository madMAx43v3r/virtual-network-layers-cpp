/*
 * Buffer.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_STREAMBUFFER_H_
#define INCLUDE_IO_STREAMBUFFER_H_

#include <string.h>
#include <vnl/Memory.h>
#include <vnl/Util.h>
#include <vnl/io/Error.h>
#include <vnl/io/Stream.h>


namespace vnl { namespace io {

class InputBuffer {
public:
	InputBuffer(InputStream* in) : in(in) {
		buf = vnl::Page::alloc();
	}
	
	InputBuffer(const InputBuffer&) = delete;
	
	~InputBuffer() {
		buf->free();
	}
	
	void read(void* dst, int len) {
		while(len) {
			int left = limit - pos;
			if(!left) {
				pos = 0;
				limit = in->read(buf->mem, Page::size);
				if(limit <= 0) {
					return;
				}
				left = limit;
				num_read += limit;
			}
			int n = std::min(len, left);
			vnl::memcpy(dst, buf->mem + pos, n);
			dst = (char*)dst + n;
			len -= n;
			pos += n;
		}
		return;
	}
	
	int64_t get_num_read() const {
		return num_read;
	}
	
	int64_t get_input_pos() const {
		return num_read - limit + pos;
	}
	
	void reset() {
		num_read = 0;
		pos = 0;
		limit = 0;
		in->reset();
	}
	
	int error() const {
		return in->error();
	}
	
	void set_error(int err_) {
		in->set_error(err_);
	}
	
protected:
	InputStream* in = 0;
	Page* buf;
	int64_t num_read = 0;
	int pos = 0;
	int limit = 0;
	
};


class OutputBuffer {
public:
	OutputBuffer(OutputStream* out) : out(out) {
		buf = vnl::Page::alloc();
	}
	
	OutputBuffer(const InputBuffer&) = delete;
	
	~OutputBuffer() {
		buf->free();
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
			vnl::memcpy(buf->mem + pos, src, n);
			src = (const char*)src + n;
			len -= n;
			pos += n;
		}
		return;
	}
	
	int64_t get_num_write() const {
		return num_write;
	}
	
	int64_t get_output_pos() const {
		return num_write + pos;
	}
	
	void reset() {
		num_write = 0;
		pos = 0;
		out->reset();
	}
	
	bool flush() {
		if(pos) {
			if(!out->write(buf->mem, pos)) {
				return false;
			}
			num_write += pos;
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
	int64_t num_write = 0;
	int pos = 0;
	
};


} // io
} // vnl

#endif /* INCLUDE_IO_STREAMBUFFER_H_ */
