/*
 * TcpServer.cpp
 *
 *  Created on: Jan 12, 2016
 *      Author: MWITTAL
 */

namespace vnl {

class TcpServer::Proxy : public vnl::Node {
public:
	
	
	uint64_t srcmac;
	
};

TcpServer::TcpServer(int port)
	:	port(port), state(this), stream(&sock)
{
	tid_reader = launch(std::bind(&TcpServer::reader, this));
}

TcpServer::~TcpServer() {
	cancel(tid_reader);
	for(auto msg : sndbuf) {
		delete msg;
	}
}

void TcpServer::handle(phy::Message* msg) {
	Node::handle(msg);
	if(msg->mid == receive_t::id) {
		receive_t* packet = (receive_t*)msg;
		if(msg->src) {
			pending[msg->src->mac | msg->seq] = packet;
		}
		write(packet);
	} else if(msg->mid == acksig_t::id) {
		state.check();
		ByteBuffer buf(&stream);
		buf.putInt(acksig_t::id);
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
	msg->serialize(&stream);
	stream.flush();
}

void TcpServer::reader() {
	auto callback = [ackbuf](phy::Message* msg) {
		if(ackbuf.empty()) {
			phy::Object::receive(new acksig_t(this, 0, true));
		}
		ackbuf.push_back((Uplink::send_t*)msg);
	};
	while(true) {
		sock.create();
		sock.bind(port);
		sock.listen();
		stream.clear();
		for(auto it : pending) {
			write(it.second);
		}
		state.set();
		ByteBuffer buf(&stream);
		while(true) {
			uint32_t mid = buf.getInt();
			if(mid == Uplink::send_t::mid) {
				uint64_t srcmac = buf.getLong();
				Uplink::send_t* msg;
				if(sndbuf.empty()) {
					msg = new Uplink::send_t();
				} else {
					msg = sndbuf.back();
					sndbuf.pop_back();
				}
				if(buf.error || !msg->deserialize(&stream)) {
					printf("ERROR: TcpServer::reader(): msg->deserialize() failed!\n");
					sndbuf.push_back(msg);
					break;
				}
				auto it = route.find(srcmac);
				if(it != route.end()) {
					phy::Object* dst = it->second;
					msg->dst = dst;
					msg->async = true;
					msg->callback = callback;
					phy::Object::send(msg);
				} else {
					sndbuf.push_back(msg);
				}
			} else if(mid == acksig_t::id) {
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
