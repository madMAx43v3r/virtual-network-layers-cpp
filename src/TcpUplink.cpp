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
	:	endpoint(endpoint), port(port), node(0), state(this), stream(&sock)
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
	switch(msg->mid) {
	case connect_t::mid:
		node = msg->src;
		msg->ack();
		break;
	case disconnect_t::mid:
		if(msg->src == node) {
			node = 0;
		}
		msg->ack();
		break;
	case send_t::id:
		send_t* packet = (send_t*)msg;
		if(msg->src) {
			pending[msg->src->mac | msg->seq] = packet;
		}
		write(packet);
		break;
	case acksig_t::id:
		state.check();
		ByteBuffer buf(&stream);
		buf.putInt(acksig_t::id);
		buf.putInt(ackbuf.size());
		for(auto msg : ackbuf) {
			buf.putLong(msg->dst->mac | msg->seq);
			sndbuf.push_back(msg);
		}
		stream.flush();
		ackbuf.clear();
		msg->ack();
		break;
	}
}

void TcpUplink::write(send_t* msg) {
	state.check();
	ByteBuffer buf(&stream);
	buf.putInt(msg->mid);
	msg->serialize(&stream);
	stream.flush();
}

void TcpUplink::reader() {
	auto callback = [ackbuf](phy::Message* msg) {
		if(ackbuf.empty()) {
			phy::Object::receive(new acksig_t(this, 0, true));
		}
		ackbuf.push_back((receive_t*)msg);
	};
	while(true) {
		sock.create();
		sock.connect(endpoint, port);
		stream.clear();
		for(auto it : pending) {
			write(it.second);
		}
		state.set();
		ByteBuffer buf(&stream);
		while(true) {
			uint32_t mid = buf.getInt();
			if(mid == receive_t::mid) {
				receive_t* msg;
				if(sndbuf.empty()) {
					msg = new receive_t();
				} else {
					msg = sndbuf.back();
					sndbuf.pop_back();
				}
				if(buf.error || !msg->deserialize(&stream)) {
					printf("ERROR: TcpUplink::reader(): msg->deserialize() failed!\n");
					sndbuf.push_back(msg);
					break;
				}
				if(node) {
					msg->dst = node;
					msg->async = true;
					msg->callback = callback;
					phy::Object::send(msg);
				} else {
					sndbuf.push_back(msg);
				}
			} else if(mid == acksig_t::id) {
				int32_t num = buf.getInt();
				for(int i = 0; i < num; ++i) {
					uint64_t hash = buf.getLong();
					auto iter = pending.find(hash);
					if(iter != pending.end()) {
						iter->second->ack();
						pending.erase(iter);
					}
				}
			}
		}
		state.reset();
		sock.close();
	}
}


}
