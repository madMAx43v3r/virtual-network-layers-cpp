/*
 * Buffer.h
 *
 *  Created on: Jan 8, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_IO_BUFFER_H_
#define INCLUDE_IO_BUFFER_H_

#include "io/Stream.h"
#include "phy/Memory.h"

namespace vnl { namespace io {

class Buffer : public vnl::io::Stream {
public:
	Buffer() : Buffer(phy::Page::alloc()) {}
	
	Buffer(phy::Page* data) : first(data), pos(0) {
		buf = first;
	}
	
	void reset() {
		buf = first;
		pos = 0;
	}
	
	void flip() {
		buf = first;
		pos = 0;
	}
	
	virtual bool read(void* dst, int len) override {
		while(len) {
			int left = phy::Page::size - pos;
			if(!left) {
				if(!buf->next) {
					return false;
				}
				buf = buf->next;
				left = phy::Page::size;
			}
			int n = std::min(len, left);
			memcpy(dst, buf->mem + pos, n);
			pos += n;
			len -= n;
			dst = (char*)dst + n;
		}
		return true;
	}
	
	virtual bool write(const void* src, int len) override {
		while(len) {
			int left = phy::Page::size - pos;
			if(!left) {
				if(!buf->next) {
					buf->next = phy::Page::alloc();
				}
				buf = buf->next;
				left = phy::Page::size;
			}
			int n = std::min(len, left);
			memcpy(buf->mem + pos, src, n);
			pos += n;
			len -= n;
			src = (const char*)src + n;
		}
		return true;
	}
	
	virtual bool flush() override {
		return true;
	}
	
private:
	phy::Page* buf;
	phy::Page* first;
	int pos;
	
};


}}

#endif /* INCLUDE_IO_BUFFER_H_ */
