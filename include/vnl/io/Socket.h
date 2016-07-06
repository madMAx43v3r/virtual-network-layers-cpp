/*
 * Socket.h
 *
 *  Created on: Jul 2, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_SOCKET_H_
#define INCLUDE_VNL_SOCKET_H_

#include <vnl/io/Stream.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


namespace vnl { namespace io {

/*
 * This class must be thread-safe !!!
 */
class Socket : public InputStream, public OutputStream {
public:
	Socket(int sock) : sock(sock) {}
	
	int get_fd() const {
		return sock;
	}
	
	virtual int read(void* dst, int len) {
		return ::read(sock, dst, len);
	}
	
	virtual bool write(const void* src, int len) {
		while(len > 0) {
			int res = ::write(sock, src, len);
			if(res > 0) {
				len -= res;
				src = (char*)src + res;
			} else {
				return false;
			}
		}
		return true;
	}
	
	void close() {
		::close(fd);
	}
	
private:
	int sock;
	
};



}}

#endif /* INCLUDE_VNL_SOCKET_H_ */
