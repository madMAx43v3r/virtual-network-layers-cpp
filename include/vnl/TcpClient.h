/*
 * TcpClient.h
 *
 *  Created on: Jul 3, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_TCPCLIENT_H_
#define CPP_INCLUDE_VNI_TCPCLIENT_H_

#include <vnl/TcpClientSupport.hxx>


namespace vnl {

class TcpClient : public TcpClientBase {
public:
	TcpClient(const vnl::String& topic, vnl::String endpoint = "", int port = 0)
		:	TcpClientBase(local_domain_name, topic), connected(false)
	{
		if(endpoint.size()) {
			this->endpoint = endpoint;
		}
		if(port) {
			this->port = port;
		}
	}
	
protected:
	int connect() {
		int sock = -1;
		while(!autoclose || !connected) {
			connected = false;
			if(sock >= 0) {
				::close(sock);
				usleep(error_interval);
			}
			if(!dorun) {
				sock = -1;
				break;
			}
			sock = ::socket(AF_INET, SOCK_STREAM, 0);
			if(sock < 0) {
				log(ERROR).out << "Failed to create client socket, error=" << errno << vnl::endl;
				usleep(error_interval*10);
				continue;
			}
			if(setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &send_buffer_size, sizeof(send_buffer_size)) < 0) {
				log(WARN).out << "setsockopt() for send_buffer_size failed, error=" << errno << vnl::endl;
			}
			if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &receive_buffer_size, sizeof(receive_buffer_size)) < 0) {
				log(WARN).out << "setsockopt() for receive_buffer_size failed, error=" << errno << vnl::endl;
			}
			int value = tcp_nodelay ? 1 : 0;
			if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value)) < 0) {
				log(WARN).out << "setsockopt() for tcp_nodelay failed, error=" << errno << vnl::endl;
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
			int err = ::connect(sock, (sockaddr*)&addr, sizeof(addr));
			if(err < 0) {
				log(DEBUG).out << "Could not connect to " << endpoint << ", error=" << errno << vnl::endl;
				continue;
			}
			connected = true;
			break;
		}
		return sock;
	}
	
private:
	bool connected;
	
};


}

#endif /* CPP_INCLUDE_VNI_TCPCLIENT_H_ */
