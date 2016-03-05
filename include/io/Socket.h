/*
 * Socket.h
 *
 *  Created on: Jan 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_SOCKET_H_
#define INCLUDE_IO_SOCKET_H_

#include "phy/Object.h"
#include "io/Stream.h"
#include "io/socket/Server.h"

namespace vnl { namespace io {

class Socket {
public:
	Socket(vnl::phy::Object* obj);
	
	int create();
	int close();
	int connect(const std::string& endpoint, int port);
	int bind(int port);
	int listen(int backlog = 100);
	Socket* accept();
	
	int read(void* buf, int len);
	bool write(const void* buf, int len);
	
protected:
	bool poll(phy::Stream& stream, int flag);
	void update(phy::Stream& stream);
	
private:
	vnl::io::socket::Server* server;
	vnl::io::socket::Server::key_t key;
	
	vnl::phy::Object* object;
	
	phy::Stream in;
	phy::Stream out;
	
};

}}

#endif /* INCLUDE_IO_SOCKET_H_ */
