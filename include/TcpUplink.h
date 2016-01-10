/*
 * TcpUplink.h
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#ifndef INCLUDE_TCP_TCPUPLINK_H_
#define INCLUDE_TCP_TCPUPLINK_H_

#include <vector>

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
	void handle(phy::Message* msg) override;
	
	void reader();
	void writer();
	
private:
	std::string endpoint;
	int port;
	uint64_t tid_reader;
	uint64_t tid_writer;
	io::Socket sock;
	phy::Condition state;
	phy::Stream queue;
	
};


}

#endif /* INCLUDE_TCP_TCPUPLINK_H_ */
