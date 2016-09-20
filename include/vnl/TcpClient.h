/*
 * TcpClient.h
 *
 *  Created on: Jul 3, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_TCPCLIENT_H_
#define CPP_INCLUDE_VNI_TCPCLIENT_H_

#include <vnl/Downlink.h>
#include <vnl/TcpClientSupport.hxx>


namespace vnl {

class TcpClient : public TcpClientBase {
public:
	TcpClient(vnl::String endpoint, int port = 8916)
		:	TcpClientBase(local_domain_name, vnl::String() << "vnl/tcp/client/" << endpoint << "/" << port)
	{
		this->endpoint = endpoint;
		this->port = port;
	}
	
protected:
	virtual void reset() {
		connect();
		if(sock.good()) {
			Super::reset();
		}
	}
	
	virtual int32_t get_fd() const {
		return sock.fd;
	}
	
	void connect() {
		while(dorun) {
			if(sock.good()) {
				::close(sock.fd);
				usleep(error_interval*1000);
			}
			if(!dorun) {
				sock = -1;
				break;
			}
			sock = ::socket(AF_INET, SOCK_STREAM, 0);
			if(!sock.good()) {
				log(ERROR).out << "Failed to create client socket, error=" << errno << vnl::endl;
				usleep(error_interval*10*1000);
				continue;
			}
			if(setsockopt(sock.fd, SOL_SOCKET, SO_SNDBUF, &send_buffer_size, sizeof(send_buffer_size)) < 0) {
				log(WARN).out << "setsockopt() for send_buffer_size failed, error=" << errno << vnl::endl;
			}
			if(setsockopt(sock.fd, SOL_SOCKET, SO_RCVBUF, &receive_buffer_size, sizeof(receive_buffer_size)) < 0) {
				log(WARN).out << "setsockopt() for receive_buffer_size failed, error=" << errno << vnl::endl;
			}
			sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			hostent* host = gethostbyname(endpoint.to_string().c_str());
			if(host) {
				memcpy(&addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
			} else {
				log(ERROR).out << "Could not resolve " << endpoint << vnl::endl;
				continue;
			}
			log(INFO).out << "Connecting to " << endpoint << ":" << port << vnl::endl;
			int err = ::connect(sock.fd, (sockaddr*)&addr, sizeof(addr));
			if(err < 0) {
				log(DEBUG).out << "Could not connect to " << endpoint << ", error=" << errno << vnl::endl;
				continue;
			}
			break;
		}
	}
	
};



}

#endif /* CPP_INCLUDE_VNI_TCPCLIENT_H_ */
