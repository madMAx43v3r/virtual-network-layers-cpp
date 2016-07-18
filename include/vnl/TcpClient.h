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
		:	TcpClientBase(local_domain_name, vnl::String() << "vnl/tcp/client/" << endpoint),
			fd(-1)
	{
		this->endpoint = endpoint;
		this->port = port;
	}
	
protected:
	virtual void main(vnl::Engine* engine, vnl::Message* init) {
		Downlink* downlink = new Downlink(my_domain, vnl::String(my_topic) << "/downlink");
		downlink->uplink.set_address(my_address);
		vnl::spawn(downlink);
		init->ack();
		run();
		Downlink::close_t close;
		send(&close, downlink);
	}
	
	virtual void reset() {
		fd = connect();
		if(fd > 0) {
			Uplink::sock = vnl::io::Socket(fd);
			Uplink::reset();
		}
	}
	
	virtual void shutdown() {
		exit();
	}
	
	virtual int32_t get_fd() const {
		return fd;
	}
	
	int32_t connect() {
		fd = -1;
		while(dorun) {
			if(fd > 0) {
				::close(fd);
			}
			fd = ::socket(AF_INET, SOCK_STREAM, 0);
			if(fd < 0) {
				log(ERROR).out << "Failed to create client socket, error=" << fd << vnl::endl;
				return -1;
			}
			sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			hostent* host = gethostbyname(endpoint.to_string().c_str());
			if(host) {
				memcpy(&addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
			} else {
				log(DEBUG).out << "Could not resolve " << endpoint << vnl::endl;
				usleep(error_interval*1000);
				continue;
			}
			log(INFO).out << "Connecting to " << endpoint << ":" << port << vnl::endl;
			int err = ::connect(fd, (sockaddr*)&addr, sizeof(addr));
			if(err < 0) {
				log(DEBUG).out << "Could not connect to " << endpoint << ", error=" << err << vnl::endl;
				usleep(error_interval*1000);
				continue;
			}
			break;
		}
		return fd;
	}
	
private:
	int fd;
	
};



}

#endif /* CPP_INCLUDE_VNI_TCPCLIENT_H_ */
