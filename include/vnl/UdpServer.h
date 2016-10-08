/*
 * UdpServer.h
 *
 *  Created on: Oct 1, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_UDPSERVER_H_
#define INCLUDE_VNL_UDPSERVER_H_

#include <vnl/UdpServerSupport.hxx>
#include <vnl/UdpClient.h>


namespace vnl {

class UdpServer : public UdpServerBase {
public:
	UdpServer(const vnl::String& topic, int port = -1)
		:	UdpServerBase(vnl::local_domain_name, topic),
			sock(-1)
	{
		if(port >= 0) {
			this->port = port;
		}
	}
	
protected:
	struct chunk_t {
		Page* data;
		int index;
	};
	
	struct packet_t {
		vnl::List<chunk_t> chunks;
		int seq_num;
		int num_pages;
	};
	
	void main() {
		
		run();
	}
	
	Page* receive(sockaddr* p_src_addr = 0) {
		Page* data = Page::alloc();
		sockaddr_storage src_addr;
		socklen_t src_addr_len = sizeof(src_addr);
		int count = recvfrom(sock, data->mem, Page::size, 0, (sockaddr*)&src_addr, &src_addr_len);
		
	}
	
private:
	int port;
	int sock;
	
	vnl::Map<sockaddr, packet_t> queue;
	
};


} // vnl

#endif /* INCLUDE_VNL_UDPSERVER_H_ */
