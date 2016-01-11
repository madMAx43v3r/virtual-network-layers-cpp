/*
 * Buffer.h
 *
 *  Created on: Jan 8, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_STREAMBUFFER_H_
#define INCLUDE_IO_STREAMBUFFER_H_

#include <string.h>
#include "io/Stream.h"

namespace vnl { namespace io {

class StreamBuffer : public vnl::io::Stream {
public:
	StreamBuffer(Stream* stream, int N = 4096) : N(N), stream(stream), in(N), out(N) {
		clear();
	}
	
	void clear() {
		in.pos = 0;
		in.left = 0;
		out.pos = 0;
		out.left = N;
	}
	
	virtual int read(void* dst, int len) override {
		int rem = len;
		while(rem > 0) {
			if(in.left > 0) {
				int n = std::min(in.left, rem);
				memcpy(dst, in.buf+in.pos, n);
				in.pos += n;
				in.left -= n;
				rem -= n;
				dst = ((char*)dst) + n;
			} else {
				int res = stream->read(in.buf, N);
				if(res <= 0) {
					return res;
				}
				in.left = res;
				in.pos = 0;
			}
		}
		return len;
	}
	
	virtual bool write(const void* src, int len) override {
		int rem = len;
		while(rem > 0) {
			if(out.left > 0) {
				int n = std::min(out.left, rem);
				memcpy(out.buf+out.pos, src, n);
				out.pos += n;
				out.left -= n;
				rem -= n;
				src = ((char*)src) + n;
			} else if(!flush()) {
				return false;
			}
		}
		return true;
	}
	
	bool flush() {
		if(out.left > 0) {
			int res = stream->write(out.buf, out.left);
			out.pos = 0;
			out.left = N;
			if(res <= 0) {
				return false;
			}
		}
		return true;
	}
	
protected:
	Stream* stream;
	
	struct buf_t {
		buf_t(int N) : pos(0), left(0) {
			buf = new char[N];
		}
		~buf_t() {
			delete [] buf;
		}
		char* buf;
		int pos;
		int left;
	};
	
private:
	int N;
	buf_t in;
	buf_t out;
	
};

}}

#endif /* INCLUDE_IO_STREAMBUFFER_H_ */
