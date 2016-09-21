/*
 * TcpServer.h
 *
 *  Created on: Jul 3, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_TCPSERVER_H_
#define CPP_INCLUDE_VNI_TCPSERVER_H_

#include <vnl/Uplink.h>
#include <vnl/TcpServerSupport.hxx>

#include <thread>


namespace vnl {

class TcpProxy : public Uplink {
public:
	TcpProxy(int fd)
		:	Uplink(local_domain_name, vnl::String() << "vnl.tcp.proxy." << fd),
			running(false)
	{
		this->fd = fd;
	}
	
protected:
	int connect() {
		if(!running) {
			running = true;
			return fd;
		} else {
			return -1;
		}
	}
	
private:
	bool running;
	int fd;
	
};


class TcpServer : public vnl::TcpServerBase {
public:
	TcpServer(int port = 8916)
		:	TcpServerBase(local_domain_name, vnl::String() << "vnl.tcp.server." << port),
			server(-1), do_reset(false)
	{
		this->port = port;
	}
	
	void receive(Message* msg) {
		Super::receive(msg);
	}
	
	typedef MessageType<int, 0xaea64cbf> new_client_t;
	typedef MessageType<int, 0x0252a160> error_t;
	
protected:
	void main() {
		while(dorun) {
			if(server >= 0) {
				::close(server);
				usleep(error_interval);
			}
			server = ::socket(AF_INET, SOCK_STREAM, 0);
			if(server < 0) {
				log(ERROR).out << "Failed to create server socket, error=" << server << vnl::endl;
				usleep(error_interval*10);
				continue;
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
			err = ::listen(server, accept_queue);
			if(err < 0) {
				log(ERROR).out << "Failed to listen on port " << port << ", error=" << errno << vnl::endl;
				continue;
			}
			log(INFO).out << "Running on port=" << port << vnl::endl;
			std::thread thread(std::bind(&TcpServer::accept_loop, this));
			while(poll(-1)) {
				if(do_reset) {
					do_reset = false;
					usleep(error_interval);
					break;
				}
			}
			::shutdown(server, SHUT_RDWR);
			thread.join();
			::close(server);
			server = -1;
		}
	}
	
	bool handle(Message* msg) {
		if(Super::handle(msg)) {
			return true;
		}
		if(msg->msg_id == new_client_t::MID) {
			int sock = ((new_client_t*)msg)->data;
			if(setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &send_buffer_size, sizeof(send_buffer_size)) < 0) {
				log(WARN).out << "setsockopt() for send_buffer_size failed, error=" << errno << vnl::endl;
			}
			if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &receive_buffer_size, sizeof(receive_buffer_size)) < 0) {
				log(WARN).out << "setsockopt() for receive_buffer_size failed, error=" << errno << vnl::endl;
			}
			TcpProxy* proxy = new TcpProxy(sock);
			vnl::spawn(proxy);
			log(INFO).out << "New client on socket " << sock << vnl::endl;
		} else if(msg->msg_id == error_t::MID) {
			log(ERROR).out << "accept() failed, error=" << ((error_t*)msg)->data << vnl::endl;
			do_reset = true;
		}
		return false;
	}
	
	void publish(const vnl::Topic& topic) {
		publish(topic.domain, topic.name);
	}
	
	void publish(const vnl::String& domain, const vnl::String& topic) {
		// TODO
	}
	
private:
	void accept_loop() {
		Actor actor;
		while(dorun) {
			int sock = ::accept(server, 0, 0);
			if(sock < 0) {
				if(dorun) {
					error_t msg(errno);
					actor.send(&msg, this);
				}
				break;
			}
			new_client_t msg(sock);
			actor.send(&msg, this);
		}
	}
	
private:
	int server;
	bool do_reset;
	
};






}

#endif /* CPP_INCLUDE_VNI_TCPSERVER_H_ */
