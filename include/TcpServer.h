/*
 * TcpServer.h
 *
 *  Created on: Jan 12, 2016
 *      Author: MWITTAL
 */

#ifndef INCLUDE_TCPSERVER_H_
#define INCLUDE_TCPSERVER_H_

#include <unordered_map>
#include <vector>

#include "Node.h"
#include "Uplink.h"
#include "io/Socket.h"
#include "io/StreamBuffer.h"

namespace vnl {

class TcpServer : phy::Object {
public:
	TcpServer(Uplink* uplink, int port);
	
protected:
	typedef phy::Signal<0x1fa3586f> acksig_t;
	
	void acceptor();
	
	class Proxy;
	
private:
	int port;
	Uplink* uplink;
	io::Socket sock;
	uint64_t tid_acceptor;
	
};

}

#endif /* INCLUDE_TCPSERVER_H_ */
