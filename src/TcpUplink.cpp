/*
 * TcpUplink.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include "TcpUplink.h"
#include "io/Buffer.h"
#include "io/BufferedStream.h"

namespace vnl {

TcpUplink::TcpUplink(const std::string& endpoint, int port)
	:	endpoint(endpoint), port(port), state(this)
{
	tid_reader = launch(std::bind(&TcpUplink::reader, this));
	tid_writer = launch(std::bind(&TcpUplink::writer, this));
}

TcpUplink::~TcpUplink() {
	cancel(tid_reader);
	cancel(tid_writer);
	for(auto msg : queue) {
		msg->ack();
	}
}

void TcpUplink::handle(phy::Message* msg) {
	Uplink::handle(msg);
	if(msg->mid == Uplink::send_t::mid) {
		queue.push((Uplink::send_t*)msg);
	}
}

void TcpUplink::reader() {
	while(true) {
		sock.create();
		sock.connect(endpoint, port);
		state.set();
		io::BufferedStream stream(sock);
		phy::SendBuffer<Node::receive_t, 100> buf;
		while(true) {
			Node::receive_t* msg = buf.get();
			if(!msg->deserialize(&stream)) {
				printf("ERROR: TcpUplink::reader(): msg->deserialize() failed!\n");
				break;
			}
			msg->dst = get_node(msg->frame.dst.B);
			if(msg->dst) {
				
			} else {
				delete msg;
			}
		}
		state.reset();
		sock.close();
	}
}

void TcpUplink::writer() {
	
}


}
