/*
 * TcpUplink.h
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#ifndef INCLUDE_TCPUPLINK_H_
#define INCLUDE_TCPUPLINK_H_

#include <vector>
#include <unordered_map>

#include "Node.h"
#include "Uplink.h"
#include "io/Socket.h"
#include "io/StreamBuffer.h"

namespace vnl {

class TcpUplink : public Uplink {
public:
	TcpUplink(const std::string& endpoint, int port);
	~TcpUplink();
	
protected:
	typedef phy::Signal<0x1fa3586f> acksig_t;
	
	virtual bool handle(phy::Message* msg) override;
	void write(send_t* msg);
	
	void reader();
	
private:
	std::string endpoint;
	int port;
	phy::Object* node;
	
	phy::taskid_t tid_reader;
	phy::Condition upstate;
	phy::Condition downstate;
	io::Socket sock;
	io::StreamBuffer stream;
	
	std::unordered_map<uint64_t, send_t*> pending;
	std::vector<receive_t*> ackbuf;
	std::vector<receive_t*> sndbuf;
	
};


}

#endif /* INCLUDE_TCPUPLINK_H_ */
