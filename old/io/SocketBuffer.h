/*
 * Buffer.h
 *
 *  Created on: Jan 8, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_SOCKETBUFFER_H_
#define INCLUDE_IO_SOCKETBUFFER_H_

#include <string.h>
#include "io/Stream.h"
#include "io/Socket.h"

namespace vnl { namespace io {

class SocketBuffer : public vnl::io::Stream {
public:
	SocketBuffer(Socket* sock, int size = 4096) : N(size), sock(sock), in(N), out(N) {
		clear();
	}
	
	void clear() {
		in.pos = 0;
		in.left = 0;
		out.pos = 0;
		out.left = N;
	}
	
	virtual bool read(void* dst, int len) override {
		while(len > 0) {
			if(in.left > 0) {
				int n = std::min(in.left, len);
				memcpy(dst, in.buf+in.pos, n);
				in.pos += n;
				in.left -= n;
				len -= n;
				dst = ((char*)dst) + n;
			} else if(len >= N) {
				int res = sock->read(dst, len);
				if(res <= 0) {
					return false;
				}
				len -= res;
				dst = ((char*)dst) + res;
			} else {
				int res = sock->read(in.buf, N);
				if(res <= 0) {
					return false;
				}
				in.left = res;
				in.pos = 0;
			}
		}
		return true;
	}
	
	virtual bool write(const void* src, int len) override {
		if(len >= N) {
			if(out.pos && !flush()) {
				return false;
			}
			int res = sock->write(src, len);
			if(res <= 0) {
				return false;
			} else {
				return true;
			}
		}
		while(len > 0) {
			if(out.left > 0) {
				int n = std::min(out.left, len);
				memcpy(out.buf+out.pos, src, n);
				out.pos += n;
				out.left -= n;
				len -= n;
				src = ((const char*)src) + n;
			} else if(!flush()) {
				return false;
			}
		}
		return true;
	}
	
	virtual bool flush() override {
		if(out.pos > 0) {
			int res = sock->write(out.buf, out.pos);
			out.pos = 0;
			out.left = N;
			if(res <= 0) {
				return false;
			}
		}
		return true;
	}
	
protected:
	Socket* sock;
	
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

#endif /* INCLUDE_IO_SOCKETBUFFER_H_ */
