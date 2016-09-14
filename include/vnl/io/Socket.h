/*
 * Socket.h
 *
 *  Created on: Jul 2, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_SOCKET_H_
#define INCLUDE_VNL_SOCKET_H_

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <vnl/io/Stream.h>


namespace vnl { namespace io {

class Socket : public InputStream, public OutputStream {
public:
	int fd;
	
	Socket() : fd(-1) {}
	
	Socket(int sock) : fd(sock) {}
	
	virtual int read(void* dst, int len) {
		int res = ::read(fd, dst, len);
		if(res <= 0) {
			InputStream::set_error(VNL_IO_CLOSED);
		}
		return res;
	}
	
	virtual bool write(const void* src, int len) {
		while(len > 0) {
			int res = ::send(fd, src, len, MSG_NOSIGNAL);
			if(res > 0) {
				len -= res;
				src = (char*)src + res;
			} else {
				OutputStream::set_error(VNL_IO_CLOSED);
				return false;
			}
		}
		return true;
	}
	
	void close() {
		::close(fd);
	}
	
};



}}

#endif /* INCLUDE_VNL_SOCKET_H_ */
