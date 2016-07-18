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
		init->ack();
		Downlink* downlink = new Downlink(my_domain, vnl::String(my_topic) << "/downlink");
		downlink->uplink.set_address(my_address);
		vnl::spawn(downlink);
		run();
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
		// TODO: this will never be called
		::close(server);
		Object::receive(msg);
	}
	
protected:
	virtual void main(vnl::Engine* engine, vnl::Message* init) {
		init->ack();
		signal(SIGPIPE, SIG_IGN);	// avoid SIGPIPE
		while(dorun) {
			if(server > 0) {
				usleep(error_interval*1000);
			}
			server = ::socket(AF_INET, SOCK_STREAM, 0);
			if(server < 0) {
				log(ERROR).out << "Failed to create server socket, error=" << server << vnl::endl;
				return;
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
					log(ERROR).out << "accept() returned error " << errno << vnl::endl;
					break;
				}
				vnl::spawn(new TcpProxy(sock));
				log(INFO).out << "New client on socket " << sock << vnl::endl;
				poll(0);
			}
			::close(server);
		}
	}
	
private:
	int server;
	
};






}

#endif /* CPP_INCLUDE_VNI_TCPSERVER_H_ */
