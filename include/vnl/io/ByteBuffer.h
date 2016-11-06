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

template<typename TPage, int N = TPage::size>
class BasicByteBuffer : public InputStream, public OutputStream {
public:
	BasicByteBuffer() : BasicByteBuffer(0) {}
	
	BasicByteBuffer(TPage* begin) {
		wrap(begin);
	}
	
	BasicByteBuffer(TPage* begin, int size) {
		wrap(begin, size);
	}
	
	void wrap(TPage* begin) {
		first = begin;
		reset();
	}
	
	void wrap(TPage* begin, int size) {
		wrap(begin);
		lim = size;
	}
	
	void reset() {
		buf = first;
		lim = 0;
		pos = 0;
		off = 0;
		InputStream::err = 0;
		OutputStream::err = 0;
	}
	
	void clear() {
		first = 0;
		reset();
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
		int left = N - off;
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
			left = std::min(N, max_left);
		}
		int n = std::min(len, left);
		memcpy(dst, buf->mem + off, n);
		pos += n;
		off += n;
		return n;
	}
	
	virtual bool write(const void* src, int len) {
		assert(buf);
		while(len) {
			int left = N - off;
			if(!left) {
				if(!buf->next) {
					buf->next = TPage::alloc();
				}
				buf = buf->next;
				off = 0;
				left = N;
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
	TPage* buf;
	TPage* first;
	int lim = 0;
	int pos = 0;
	int off = 0;
	
};

typedef BasicByteBuffer<vnl::Page> ByteBuffer;


} // io
} // vnl

#endif /* INCLUDE_IO_PAGEBUFFER_H_ */
