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
	
	int read(void* dst, int len) override {
		int n = std::min(limit-pos, len);
		memcpy(dst, buf+pos, n);
		pos += n;
		return n;
	}
	
	bool write(const void* src, int len) override {
		while(N-pos < len) {
			expand();
		}
		memcpy(buf+pos, src, len);
		pos += len;
		return true;
	}
	
protected:
	void expand() {
		char* old = buf;
		N *= 2;
		buf = new char[N];
		memcpy(buf, old, N/2);
		delete [] old;
	}
	
private:
	int N;
	char* buf;
	int pos;
	int limit;
	
};


}}

#endif /* INCLUDE_IO_BUFFER_H_ */
