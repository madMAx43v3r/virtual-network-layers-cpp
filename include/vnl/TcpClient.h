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
		:	TcpClientBase(local_domain_name, vnl::String() << "vnl/tcp/client/" << endpoint)
	{
		this->endpoint = endpoint;
		this->port = port;
	}
	
protected:
	virtual void main(vnl::Engine* engine, vnl::Message* init) {
		Downlink* downlink = new Downlink(my_domain, vnl::String(my_topic) << "/downlink");
		downlink->uplink.set_address(my_address);
		vnl::spawn(downlink);
		Super::main(engine, init);
		Downlink::close_t close;
		send(&close, downlink);
	}
	
	virtual void reset() {
		connect();
		if(fd > 0) {
			Super::reset();
		}
	}
	
	virtual int32_t get_fd() const {
		return fd;
	}
	
	virtual void shutdown() {
		exit();
	}
	
	void connect() {
		while(dorun) {
			if(fd > 0) {
				::close(fd);
				usleep(error_interval*1000);
			}
			fd = ::socket(AF_INET, SOCK_STREAM, 0);
			if(fd < 0) {
				log(ERROR).out << "Failed to create client socket, error=" << fd << vnl::endl;
				usleep(error_interval*10*1000);
				continue;
			}
			if(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &send_buffer_size, sizeof(send_buffer_size)) < 0) {
				log(WARN).out << "setsockopt() for send_buffer_size failed, error=" << errno << vnl::endl;
			}
			if(setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &receive_buffer_size, sizeof(receive_buffer_size)) < 0) {
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
			int err = ::connect(fd, (sockaddr*)&addr, sizeof(addr));
			if(err < 0) {
				log(DEBUG).out << "Could not connect to " << endpoint << ", error=" << err << vnl::endl;
				continue;
			}
			break;
		}
	}
	
};



}

#endif /* CPP_INCLUDE_VNI_TCPCLIENT_H_ */