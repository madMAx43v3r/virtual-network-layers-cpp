/*
 * TcpServer.cpp
 *
 *  Created on: Jan 12, 2016
 *      Author: MWITTAL
 */

namespace vnl {

TcpServer::TcpServer(int port)
	:	port(port), state(this), stream(&sock)
{
	tid_reader = launch(std::bind(&TcpUplink::reader, this));
}

TcpServer::~TcpServer() {
	cancel(tid_reader);
	for(auto msg : sndbuf) {
		delete msg;
	}
}

void TcpServer::handle(phy::Message* msg) {
	Uplink::handle(msg);
	if(msg->mid == send_t::mid) {
		send_t* packet = (send_t*)msg;
		if(msg->src) {
			pending[msg->src->mac | msg->seq] = packet;
		}
		write(packet);
	} else if(msg->mid == acksig_t) {
		state.check();
		ByteBuffer buf(&stream);
		buf.putInt(ackid);
		for(auto msg : ackbuf) {
			buf.putLong(msg->dst->mac | msg->seq);
			sndbuf.push_back(msg);
		}
		stream.flush();
		msg->ack();
	}
}

void TcpServer::write(receive_t* msg) {
	state.check();
	ByteBuffer buf(&stream);
	buf.putInt(msg->mid);
	if(msg->src) {
		buf.putLong(msg->src->mac);
	} else {
		buf.putLong(0);
	}
	msg->serialize(&stream);
	stream.flush();
}

void TcpServer::reader() {
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
					sndbuf.push_back(msg);
					break;
				}
				phy::Object* dst = get_node(dstmac);
				if(dst) {
					msg->dst = dst;
					msg->async = true;
					msg->callback = callback;
					phy::Object::send(msg);
				} else {
					sndbuf.push_back(msg);
				}
			} else if(mid == ackid) {
				uint64_t hash = buf.getLong();
				auto iter = pending.find(hash);
				if(iter != pending.end()) {
					iter->second->ack();
					pending.erase(iter);
				}
			}
		}
		state.reset();
		sock.close();
	}
}


}
