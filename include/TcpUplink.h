/*
 * TcpUplink.h
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#ifndef INCLUDE_TCP_TCPUPLINK_H_
#define INCLUDE_TCP_TCPUPLINK_H_

#include <vector>
#include <unordered_map>

#include "Uplink.h"
#include "io/Socket.h"
#include "io/Buffer.h"
#include "util/simple_queue.h"

namespace vnl {

class TcpUplink : public Uplink {
public:
	TcpUplink(const std::string& endpoint, int port);
	~TcpUplink();
	
protected:
	virtual void handle(phy::Message* msg) override;
	
	void write(send_t* msg);
	
	void reader();
	
private:
	std::string endpoint;
	int port;
	
	uint64_t tid_reader;
	phy::Condition state;
	io::Socket sock;
	io::StreamBuffer stream;
	
	std::unordered_map<uint64_t, send_t*> pending;
	std::vector<receive_t*> sndbuf;
	
	static const uint32_t ackid = 0xf641c12f;
	
};


}

#endif /* INCLUDE_TCP_TCPUPLINK_H_ */
