/*
 * TcpServer.h
 *
 *  Created on: Jul 3, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_TCPSERVER_H_
#define CPP_INCLUDE_VNI_TCPSERVER_H_

#include <sys/signal.h>

#include <vnl/Downlink.h>
#include <vnl/Uplink.h>
#include <vnl/TcpServerSupport.hxx>


namespace vnl {

class TcpProxy : public Uplink {
public:
	TcpProxy(int fd)
		:	Uplink(local_domain_name, vnl::String() << "vnl/tcp/proxy/" << fd),
			running(false), error(false)
	{
		sock = vnl::io::Socket(fd);
	}
	
protected:
	virtual void main(vnl::Engine* engine, vnl::Message* init) {
		Downlink* downlink = new Downlink(my_domain, vnl::String(my_topic) << "/downlink");
		downlink->uplink.set_address(my_address);
		vnl::spawn(downlink);
		Uplink::main(engine, init);
		Downlink::close_t close;
		send(&close, downlink);
	}
	
	virtual void reset() {
		if(!running) {
			running = true;
		} else {
			error = true;
		}
	}
	
	virtual void shutdown() {
		exit();
	}
	
	virtual int32_t get_fd() const {
		if(!error) {
			return sock.get_fd();
		} else {
			return -1;
		}
	}
	
private:
	bool running;
	bool error;
	
};


class TcpServer : public vnl::TcpServerBase {
public:
	TcpServer(int port = 8916)
		:	TcpServerBase(local_domain_name, vnl::String() << "vnl/tcp/server/" << port),
			server(-1)
	{
		this->port = port;
	}
	
	virtual void receive(vnl::Message* msg) {
		if(Layer::shutdown) {
			dorun = false;
			::close(server);
		}
		Super::receive(msg);
	}
	
protected:
	virtual void main(vnl::Engine* engine, vnl::Message* init) {
		init->ack();
		while(dorun) {
			if(server > 0) {
				usleep(error_interval*1000);
			}
			server = ::socket(AF_INET, SOCK_STREAM, 0);
			if(server < 0) {
				log(ERROR).out << "Failed to create server socket, error=" << server << vnl::endl;
				return;
			}
			int enable = 1;
			if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
				log(ERROR).out << "setsockopt(SO_REUSEADDR) failed, error=" << errno << vnl::endl;
			}
			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = INADDR_ANY;
			int err = ::bind(server, (sockaddr*)&addr, sizeof(addr));
			if(err < 0) {
				log(ERROR).out << "Failed to bind server socket to port " << port << ", error=" << errno << vnl::endl;
				continue;
			}
			err = ::listen(server, 10);
			if(err < 0) {
				log(ERROR).out << "Failed to listen on port " << port << ", error=" << errno << vnl::endl;
				continue;
			}
			log(INFO).out << "Running on port=" << port << vnl::endl;
			while(dorun) {
				int sock = ::accept(server, 0, 0);
				if(sock < 0) {
					log(ERROR).out << "accept() failed, error=" << errno << vnl::endl;
					break;
				}
				if(setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &send_buffer_size, sizeof(send_buffer_size)) < 0) {
					log(WARN).out << "setsockopt() for send_buffer_size failed, error=" << errno << vnl::endl;
				}
				if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &receive_buffer_size, sizeof(receive_buffer_size)) < 0) {
					log(WARN).out << "setsockopt() for receive_buffer_size failed, error=" << errno << vnl::endl;
				}
				vnl::spawn(new TcpProxy(sock));
				log(INFO).out << "New client on socket " << sock << vnl::endl;
			}
			::close(server);
		}
	}
	
	virtual void publish(const vnl::Topic& topic) {
		// TODO
	}
	
	virtual void publish(const vnl::String& domain, const vnl::String& topic) {
		// TODO
	}
	
private:
	int server;
	
};






}

#endif /* CPP_INCLUDE_VNI_TCPSERVER_H_ */