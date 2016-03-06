/*
 * TcpUplink.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include <io/SocketBuffer.h>
#include "TcpUplink.h"

namespace vnl {

TcpUplink::TcpUplink(const std::string& endpoint, int port)
	:	Uplink::Uplink(this),
		endpoint(endpoint), port(port), node(0), stream(&sock)
{
	launch(std::bind(&TcpUplink::reader, this));
}

TcpUplink::~TcpUplink() {
	
}

bool TcpUplink::handle(phy::Message* msg) {
	switch(msg->mid) {
	case connect_t::id:
		node = (Node*)msg->src;
		downstate.set();
		msg->ack();
		return true;
	case disconnect_t::id:
		if(msg->src == node) {
			downstate.reset();
			node = 0;
		}
		msg->ack();
		return true;
	case send_t::id: {
		send_t* packet = (send_t*)msg;
		packet->seq = nextseq++;
		if(msg->src) {
			pending[packet->seq] = packet;
		}
		upstate.check();
		write(packet);
		return true;
	}
	case acksig_t::id:
		upstate.check();
		ByteBuffer buf(&stream);
		buf.putInt(acksig_t::id);
		buf.putInt(ackbuf.size());
		for(auto msg : ackbuf) {
			buf.putLong(msg->seq);
			sndbuf.free(msg);
		}
		stream.flush();
		ackbuf.clear();
		msg->ack();
		return true;
	}
	return false;
}

void TcpUplink::write(send_t* msg) {
	ByteBuffer buf(&stream);
	buf.putInt(msg->mid);
	msg->serialize(&stream);
	stream.flush();
}

void TcpUplink::reader() {
	auto callback = [this](phy::Message* msg) {
		if(ackbuf.empty()) {
			receive(new acksig_t(true));
		}
		ackbuf.push_back((receive_t*)msg);
	};
	while(true) {
		sock.create();
		sock.connect(endpoint, port);
		stream.clear();
		pending.clear();
		upstate.set();
		ByteBuffer buf(&stream);
		while(true) {
			uint32_t mid = buf.getInt();
			if(buf.error) {
				break;
			} else if(mid == receive_t::id) {
				receive_t* msg = sndbuf.alloc();
				if(msg->deserialize(&stream)) {
					downstate.check();
					msg->callback = callback;
					phy::Object::send(msg, node, true);
				} else {
					sndbuf.free(msg);
					break;
				}
			} else if(mid == acksig_t::id) {
				int num = buf.getInt();
				for(int i = 0; i < num; ++i) {
					uint32_t seq = buf.getInt();
					auto iter = pending.find(seq);
					if(iter != pending.end()) {
						iter->second->ack();
						pending.erase(iter);
					}
				}
			} else {
				break;
			}
		}
		upstate.reset();
		sock.close();
	}
}


}
