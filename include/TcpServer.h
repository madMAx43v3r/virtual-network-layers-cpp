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

class TcpServer : public Node {
public:
	TcpServer(int port);
	
protected:
	typedef phy::Signal<0x1fa3586f> acksig_t;
	
	virtual void handle(phy::Message* msg) override;
	void write(receive_t* msg);
	
	void reader();
	
	typedef phy::Signal<0x1fa3586f> acksig_t;
	
	class Proxy;
	
private:
	int port;
	uint64_t tid_reader;
	phy::Condition state;
	io::Socket sock;
	io::StreamBuffer stream;
	
	std::unordered_map<uint64_t, receive_t*> pending;
	std::vector<Uplink::send_t*> ackbuf;
	std::vector<Uplink::send_t*> sndbuf;
	
};

}

#endif /* INCLUDE_TCPSERVER_H_ */
