/*
 * TcpUplink.h
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#ifndef INCLUDE_TCPUPLINK_H_
#define INCLUDE_TCPUPLINK_H_

#include <io/SocketBuffer.h>
#include <vector>
#include <unordered_map>

#include "Node.h"
#include "Uplink.h"
#include "io/Socket.h"
#include "io/SocketBuffer.h"
#include "util/pool.h"

namespace vnl {

class TcpUplink : public Uplink {
public:
	TcpUplink(const std::string& endpoint, int port);
	~TcpUplink();
	
protected:
	typedef phy::SignalType<0x1fa3586f> acksig_t;
	
	virtual bool handle(phy::Message* msg) override;
	
	void write(send_t* msg);
	
	void reader();
	
private:
	std::string endpoint;
	int port;
	Node* node;
	
	phy::Condition upstate;
	phy::Condition downstate;
	
	io::Socket sock;
	io::SocketBuffer stream;
	
	std::unordered_map<uint32_t, send_t*> pending;
	std::vector<receive_t*> ackbuf;
	vnl::util::pool<receive_t> sndbuf;
	
	int32_t nextseq = 1;
	
};


}

#endif /* INCLUDE_TCPUPLINK_H_ */
