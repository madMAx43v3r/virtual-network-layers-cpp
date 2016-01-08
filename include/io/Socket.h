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

class Socket : public vnl::phy::Object, public vnl::io::Stream {
public:
	Socket(vnl::phy::Object* parent);
	
	int create();
	int close();
	int connect(const std::string& endpoint, int port);
	int bind(int port);
	int listen(int backlog = 100);
	Socket* accept();
	
	int read(void* buf, int len) override;
	bool read_all(void* buf, int len);
	
	bool write(const void* buf, int len) override;
	
protected:
	typedef vnl::phy::Generic<int, 0x98a4ad8b> signal_t;
	
	bool poll(vnl::phy::Stream& stream, int flag);
	void update();
	
private:
	vnl::io::socket::Server* server;
	vnl::io::socket::Server::key_t key;
	vnl::phy::Stream in;
	vnl::phy::Stream out;
	
};

}}

#endif /* INCLUDE_IO_SOCKET_H_ */
