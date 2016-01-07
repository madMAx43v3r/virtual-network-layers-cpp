/*
 * Socket.h
 *
 *  Created on: Jan 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_POLL_SOCKET_H_
#define INCLUDE_IO_POLL_SOCKET_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "io/socket/Server.h"

namespace vnl { namespace io {

using namespace vnl::phy;
using namespace vnl::io::socket;

class Socket : public vnl::phy::Object {
public:
	Socket(Object* parent) : Object(parent), server(Server::instance), in(this), out(this) {
		key.obj = this;
		key.sin = in.sid;
		key.sout = out.sid;
	}
	
	int create() {
		key.fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
		return key.fd;
	}
	
	int close() {
		key.obj = 0;
		update();
		return ::close(key.fd);
	}
	
	int connect(const std::string& endpoint, int port) {
		sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		hostent* host = gethostbyname(endpoint.c_str());
		if(host) {
			memcpy(&addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
		} else {
			return -1;
		}
		if(::connect(key.fd, (sockaddr*)&addr, sizeof(addr)) < 0 && errno != EINPROGRESS) {
			return -1;
		}
		if(poll(out, POLLOUT)) {
			int err;
			unsigned int len = sizeof(err);
			if(getsockopt(key.fd, SOL_SOCKET, SO_ERROR, &err, &len) == 0 && err == 0) {
				return 0;
			}
		}
		return -1;
	}
	
	int bind(int port) {
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;
		return ::bind(key.fd, (sockaddr*)&addr, sizeof(addr));
	}
	
	int listen(int backlog = 100) {
		return ::listen(key.fd, backlog);
	}
	
	Socket* accept() {
		Socket* sock = 0;
		while(true) {
			int fd = ::accept(key.fd, 0, 0);
			if(fd < 0) {
				if(!poll(in, POLLIN)) {
					return 0;
				}
			} else {
				fcntl(fd, F_SETFL, O_NONBLOCK);
				Socket* sock = new Socket(this);
				sock->key.fd = fd;
				return sock;
			}
		}
	}
	
	int read(void* buf, int len) {
		while(true) {
			int res = ::read(key.fd, buf, len);
			if(res > 0) {
				return res;
			} else if(!poll(in, POLLIN)) {
				return -1;
			}
		}
	}
	
	bool read_all(void* buf, int len) {
		int left = len;
		while(true) {
			int res = ::read(key.fd, buf, left);
			if(res > 0) {
				left -= res;
				buf = ((char*)buf) + res;
			}
			if(left > 0) {
				if(!poll(in, POLLIN)) {
					return false;
				}
			} else {
				return true;
			}
		}
	}
	
	bool write(const void* buf, int len) {
		int left = len;
		while(true) {
			int res = ::write(key.fd, buf, left);
			if(res > 0) {
				left -= res;
				buf = ((char*)buf) + res;
			}
			if(left > 0) {
				if(!poll(out, POLLOUT)) {
					return false;
				}
			} else {
				return true;
			}
		}
	}
	
protected:
	typedef Generic<int, 0x98a4ad8b> signal_t;
	
	bool poll(Stream& stream, int flag) {
		key.events |= flag;
		update();
		int err = stream.read<signal_t>().data;
		key.events &= ~flag;
		return err == 0;
	}
	
	void update() {
		key.index = request<int>(Server::poll_t(server, key));
	}
	
private:
	Server* server;
	Server::key_t key;
	Stream in;
	Stream out;
	
};

}}

#endif /* INCLUDE_IO_POLL_SOCKET_H_ */
