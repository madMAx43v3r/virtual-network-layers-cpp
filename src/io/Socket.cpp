/*
 * Socket.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "io/Socket.h"
#include "io/socket/Server.h"

using namespace vnl::phy;
using namespace vnl::io::socket;

namespace vnl { namespace io {

Server* Server::instance = 0;

Socket::Socket(vnl::phy::Object* obj) : server(Server::instance), object(obj), in(obj), out(obj) {
	assert(Server::instance != 0);
	key.obj = obj;
	key.sin = in.sid;
	key.sout = out.sid;
}

int Socket::create() {
	key.fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	return key.fd;
}

int Socket::close() {
	key.obj = 0;
	update(in);
	return ::close(key.fd);
}

int Socket::connect(const std::string& endpoint, int port) {
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

int Socket::bind(int port) {
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	return ::bind(key.fd, (sockaddr*)&addr, sizeof(addr));
}

int Socket::listen(int backlog) {
	return ::listen(key.fd, backlog);
}

Socket* Socket::accept() {
	Socket* sock = 0;
	while(true) {
		int fd = ::accept(key.fd, 0, 0);
		if(fd < 0) {
			if(!poll(in, POLLIN)) {
				return 0;
			}
		} else {
			fcntl(fd, F_SETFL, O_NONBLOCK);
			Socket* sock = new Socket(object);
			sock->key.fd = fd;
			return sock;
		}
	}
}

int Socket::read(void* buf, int len) {
	while(true) {
		int res = ::read(key.fd, buf, len);
		if(res > 0) {
			return res;
		} else if(!poll(in, POLLIN)) {
			return -1;
		}
	}
}

bool Socket::write(const void* buf, int len) {
	while(true) {
		int res = ::write(key.fd, buf, len);
		if(res > 0) {
			len -= res;
			buf = ((char*)buf) + res;
		}
		if(len > 0) {
			if(!poll(out, POLLOUT)) {
				return false;
			}
		} else {
			return true;
		}
	}
}

bool Socket::poll(phy::Stream& stream, int flag) {
	key.events |= flag;
	update(stream);
	int err = stream.read<Server::signal_t>().data;
	key.events &= ~flag;
	return err == 0;
}

void Socket::update(phy::Stream& stream) {
	key.index = stream.request<int>(Server::poll_t(key), server);
}


}}
