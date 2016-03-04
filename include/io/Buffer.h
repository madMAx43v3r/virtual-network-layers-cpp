/*
 * Buffer.h
 *
 *  Created on: Jan 8, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_IO_BUFFER_H_
#define INCLUDE_IO_BUFFER_H_

#include "io/Stream.h"

namespace vnl { namespace io {

class Buffer : public vnl::io::Stream {
public:
	Buffer(int N = 1024) : N(N), pos(0), limit(0) {
		buf = new char[N];
	}
	
	~Buffer() {
		delete [] buf;
	}
	
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	
	void reset() {
		pos = 0;
		limit = 0;
	}
	
	void flip() {
		limit = pos;
		pos = 0;
	}
	
	int size() {
		return limit;
	}
	
	void* ptr() {
		return buf;
	}
	
	void* copy() {
		void* b = new char[limit];
		memcpy(b, buf, limit);
		return b;
	}
	
	void* write(int len) {
		if(N-pos < len) {
			resize(pos+len);
		}
		void* addr = buf+pos;
		pos += len;
		return addr;
	}
	
	virtual bool read(void* dst, int len) override {
		int n = std::min(limit-pos, len);
		memcpy(dst, buf+pos, n);
		pos += n;
		return n == len;
	}
	
	virtual bool write(const void* src, int len) override {
		memcpy(write(len), src, len);
		return true;
	}
	
	virtual bool flush() override {
		return true;
	}
	
protected:
	void resize(int n) {
		char* old = buf;
		buf = new char[n];
		memcpy(buf, old, N);
		delete [] old;
		N = n;
	}
	
private:
	int N;
	char* buf;
	int pos;
	int limit;
	
};


}}

#endif /* INCLUDE_IO_BUFFER_H_ */
