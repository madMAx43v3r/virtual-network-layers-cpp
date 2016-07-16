/*
 * TcpServer.h
 *
 *  Created on: Jul 3, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_TCPSERVER_H_
#define CPP_INCLUDE_VNI_TCPSERVER_H_

#include <vnl/Downlink.h>


namespace vnl {

class TcpProxy : public Downlink {
public:
	TcpProxy(int fd)
		:	Downlink(vnl::StringWriter().out << "vni/tcp/proxy/" << vnl::dec(fd)),
			sock(fd)
	{
	}
	
protected:
	virtual vnl::io::Socket* connect() {
		if(dorun) {
			dorun = false;
			return &sock;
		} else {
			return 0;
		}
	}
	
private:
	vnl::io::Socket sock;
	bool dorun = true;
	
};


class TcpServer : public vnl::Module {
public:
	TcpServer(int port = 8916)
		:	Module(vnl::StringWriter().out << "vni/tcp/server/" << vnl::dec(port)), port(port),
			fd(-1)
	{
	}
	
	virtual void receive(vnl::Message* msg) {
		if(msg->msg_id == vnl::Registry::exit_t::MID) {
			dorun = false;
			::close(fd);
		}
		msg->ack();
	}
	
protected:
	virtual void main(vnl::Engine* engine) {
		fd = ::socket(AF_INET, SOCK_STREAM, 0);
		if(fd < 0) {
			log(ERROR).out << "Failed to create server socket, error=" << vnl::dec(fd) << vnl::endl;
			return;
		}
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;
		int err = ::bind(fd, (sockaddr*)&addr, sizeof(addr));
		if(err < 0) {
			log(ERROR).out << "Failed to bind server socket to port " << vnl::dec(port) << ", error=" << vnl::dec(err) << vnl::endl;
			return;
		}
		while(dorun) {
			int sock = ::accept(fd, 0, 0);
			if(sock < 0) {
				break;
			}
			fork(new TcpProxy(sock));
			log(INFO).out << "New client on socket " << vnl::dec(sock) << vnl::endl;
			poll(0);
		}
	}
	
private:
	volatile bool dorun = true;
	int port;
	int fd;
	
};






}

#endif /* CPP_INCLUDE_VNI_TCPSERVER_H_ */
