/*
 * TcpUplink.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include "TcpUplink.h"

#include "io/StreamBuffer.h"
#include "io/Buffer.h"

namespace vnl {

TcpUplink::TcpUplink(const std::string& endpoint, int port)
	:	endpoint(endpoint), port(port), node(0), upstate(this), downstate(this), stream(&sock)
{
	tid_reader = launch(std::bind(&TcpUplink::reader, this));
}

TcpUplink::~TcpUplink() {
	cancel(tid_reader);
	for(auto msg : sndbuf) {
		delete msg;
	}
}

bool TcpUplink::handle(phy::Message* msg) {
	if(!Uplink::handle(msg)) {
		switch(msg->mid) {
		case connect_t::mid:
			node = msg->src;
			downstate.set();
			msg->ack();
			return true;
		case disconnect_t::mid:
			if(msg->src == node) {
				downstate.reset();
				node = 0;
			}
			msg->ack();
			return true;
		case send_t::id:
			send_t* packet = (send_t*)msg;
			if(msg->src) {
				pending[msg->src->mac | msg->seq] = packet;
			}
			write(packet);
			return true;
		case acksig_t::id:
			upstate.check();
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
			return true;
		}
	}
	return false;
}

void TcpUplink::write(send_t* msg) {
	upstate.check();
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
		upstate.set();
		ByteBuffer buf(&stream);
		while(true) {
			downstate.check();
			uint32_t mid = buf.getInt();
			if(buf.error) {
				break;
			} else if(mid == receive_t::mid) {
				receive_t* msg;
				if(sndbuf.empty()) {
					msg = new receive_t();
				} else {
					msg = sndbuf.back();
					sndbuf.pop_back();
				}
				if(msg->deserialize(&stream)) {
					msg->dst = node;
					msg->async = true;
					msg->callback = callback;
					phy::Object::send(msg);
				} else {
					sndbuf.push_back(msg);
					break;
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
			} else {
				break;
			}
		}
		upstate.reset();
		sock.close();
	}
}


}
