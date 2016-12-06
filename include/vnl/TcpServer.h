/*
 * TcpServer.h
 *
 *  Created on: Jul 3, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_TCPSERVER_H_
#define CPP_INCLUDE_VNI_TCPSERVER_H_

#include <vnl/Pipe.h>
#include <vnl/TcpProxy.h>
#include <vnl/TcpServerSupport.hxx>

#include <thread>


namespace vnl {

class TcpServer : public vnl::TcpServerBase {
public:
	TcpServer(const vnl::String& domain, const vnl::String& topic)
		:	TcpServerBase(domain, topic),
			server(-1), do_reset(false), pipe(0)
	{
	}
	
	typedef MessageType<int, 0xaea64cbf> new_client_t;
	typedef MessageType<int, 0x0252a160> error_t;
	
protected:
	void main() {
		while(vnl_dorun) {
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
			pipe = Pipe::open();
			std::thread thread(std::bind(&TcpServer::accept_loop, this));
			while(poll(-1)) {
				if(do_reset) {
					do_reset = false;
					usleep(error_interval);
					break;
				}
			}
			pipe->close();
			poll(0);
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
			int value = tcp_keepalive ? 1 : 0;
			if(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value)) < 0) {
				log(WARN).out << "setsockopt() for tcp_keepalive failed, error=" << errno << vnl::endl;
			}
			value = tcp_nodelay ? 1 : 0;
			if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value)) < 0) {
				log(WARN).out << "setsockopt() for tcp_nodelay failed, error=" << errno << vnl::endl;
			}
			TcpProxy* proxy = create_proxy(sock);
			proxy->vnl_max_num_pending = vnl_max_num_pending;
			proxy->vnl_log_level = vnl_log_level;
			proxy->send_timeout = send_timeout;
			proxy->sock = sock;
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
	
	virtual TcpProxy* create_proxy(int sock) {
		return new TcpProxy(my_domain, vnl::String("vnl.tcp.proxy.") << sock);
	}
	
private:
	void accept_loop() {
		pipe->attach(this);
		ThreadEngine engine;
		Stream stream;
		stream.connect(&engine);
		while(vnl_dorun) {
			int sock = ::accept(server, 0, 0);
			if(sock < 0) {
				if(vnl_dorun) {
					error_t msg(errno);
					stream.send(&msg, pipe);
				}
				break;
			}
			new_client_t msg(sock);
			stream.send(&msg, pipe);
		}
		pipe->close();
	}
	
private:
	int server;
	bool do_reset;
	
	Pipe* pipe;
	
};


} // vnl

#endif /* CPP_INCLUDE_VNI_TCPSERVER_H_ */
