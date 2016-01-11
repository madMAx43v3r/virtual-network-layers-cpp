/*
 * TcpUplink.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include "TcpUplink.h"

#include "../include/io/StreamBuffer.h"
#include "io/Buffer.h"

namespace vnl {

TcpUplink::TcpUplink(const std::string& endpoint, int port)
	:	endpoint(endpoint), port(port), state(this), stream(&sock)
{
	tid_reader = launch(std::bind(&TcpUplink::reader, this));
}

TcpUplink::~TcpUplink() {
	cancel(tid_reader);
	for(auto msg : sndbuf) {
		delete msg;
	}
}

void TcpUplink::handle(phy::Message* msg) {
	Uplink::handle(msg);
	if(msg->mid == send_t::mid) {
		send_t* packet = (send_t*)msg;
		if(msg->src) {
			pending[msg->src->mac | msg->seq] = packet;
		}
		write(packet);
	}
}

void TcpUplink::write(send_t* msg) {
	state.check();
	ByteBuffer buf(&stream);
	if(msg->src) {
		buf.putLong(msg->src->mac);
	} else {
		buf.putLong(0);
	}
	msg->serialize(&stream);
	stream.flush();
}

void TcpUplink::reader() {
	auto release = [sndbuf](phy::Message* msg) {
		sndbuf.push_back((receive_t*)msg);
	};
	while(true) {
		sock.create();
		sock.connect(endpoint, port);
		stream.clear();
		state.set();
		for(auto it : pending) {
			write(it.second);
		}
		ByteBuffer buf(&stream);
		while(true) {
			uint64_t dstmac = buf.getLong();
			receive_t* msg;
			if(sndbuf.empty()) {
				msg = new receive_t();
			} else {
				msg = sndbuf.back();
				sndbuf.pop_back();
			}
			if(buf.error || !msg->deserialize(&stream)) {
				printf("ERROR: TcpUplink::reader(): msg->deserialize() failed!\n");
				release(msg);
				break;
			}
			phy::Object* dst = get_node(dstmac);
			if(dst) {
				msg->dst = dst;
				msg->async = true;
				msg->callback = release;
				phy::Object::send(msg);
			} else {
				release(msg);
			}
		}
		state.reset();
		sock.close();
	}
}


}
