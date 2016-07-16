/*
 * TcpClient.h
 *
 *  Created on: Jul 3, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_TCPCLIENT_H_
#define CPP_INCLUDE_VNI_TCPCLIENT_H_


#include <vnl/Downlink.h>


namespace vnl {

class TcpClient : public Downlink {
public:
	TcpClient(vnl::String endpoint, int port = 8916, vnl::String name = "default")
		:	Downlink(vnl::String() << "vni/tcp/client/" << endpoint << "/" << vnl::dec(port) << "/" << name),
			endpoint(endpoint), port(port), sock(-1)
	{
	}
	
protected:
	virtual vnl::io::Socket* connect() {
		int fd = ::socket(AF_INET, SOCK_STREAM, 0);
		if(fd < 0) {
			log(ERROR).out << "Failed to create client socket, error=" << vnl::dec(fd) << vnl::endl;
			return -1;
		}
		while(dorun) {
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
			int err = ::connect(fd, (sockaddr*)&addr, sizeof(addr));
			if(err < 0) {
				log(DEBUG).out << "Could not connect to " << endpoint << ", error=" << vnl::dec(err) << vnl::endl;
				usleep(error_interval*1000);
				continue;
			}
			break;
		}
		sock = vnl::io::Socket(fd);
		return &sock;
	}
	
private:
	vnl::String endpoint;
	int port;
	vnl::io::Socket sock;
	
};



}

#endif /* CPP_INCLUDE_VNI_TCPCLIENT_H_ */
