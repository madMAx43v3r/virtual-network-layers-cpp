/*
 * TcpServer.cpp
 *
 *  Created on: Jan 12, 2016
 *      Author: MWITTAL
 */

#include "TcpServer.h"
#include "phy/Pool.h"

namespace vnl {

class TcpServer::Proxy : Node {
public:
	Proxy(Uplink* uplink, io::Socket* sock)
		:	Node::Node(uplink),
			sock(sock), stream(sock)
	{
		launch(std::bind(&Proxy::reader, this));
	}
	
	~Proxy() {
		delete sock;
	}
	
	virtual bool handle(phy::Message* msg) override {
		if(msg->mid == receive_t::id) {
			receive_t* packet = (receive_t*)msg;
			packet->seq = nextseq++;
			if(msg->src) {
				pending[packet->seq] = packet;
			}
			write(packet);
			return true;
		} else if(msg->mid == acksig_t::id) {
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
	
	void write(receive_t* msg) {
		ByteBuffer buf(&stream);
		buf.putInt(msg->mid);
		msg->serialize(&stream);
		stream.flush();
	}
	
	void reader() {
		auto callback = [this](phy::Message* msg) {
			if(ackbuf.empty()) {
				phy::Object::receive(new acksig_t(0, true));
			}
			ackbuf.push_back((send_t*)msg);
		};
		stream.clear();
		ByteBuffer buf(&stream);
		while(true) {
			uint32_t mid = buf.getInt();
			if(buf.error) {
				break;
			} else if(mid == send_t::id) {
				uint64_t srcmac = buf.getLong();
				send_t* msg = sndbuf.alloc();
				if(msg->deserialize(&stream)) {
					const Frame& frame = msg->frame;
					msg->callback = callback;
					phy::Object::send(msg, uplink, true);
					if(frame.flags & Frame::REGISTER) {
						logical.insert(frame.dst);
					} else if(frame.flags & Frame::UNREGISTER) {
						logical.erase(frame.dst);
					}
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
		sock->close();
		exit();
	}
	
protected:
	io::Socket* sock;
	io::StreamBuffer stream;
	
	std::unordered_map<uint32_t, receive_t*> pending;
	std::vector<send_t*> ackbuf;
	phy::Pool<send_t> sndbuf;
	
	int32_t nextseq = 1;
	
};


TcpServer::TcpServer(Uplink* uplink, int port)
	:	uplink(uplink), port(port)
{
	launch(std::bind(&TcpServer::acceptor, this));
}

void TcpServer::acceptor() {
	sock.create();
	sock.bind(port);
	sock.listen();
	while(true) {
		io::Socket* conn = sock.accept();
		if(conn) {
			new Proxy(uplink, conn);
		} else {
			break;
		}
	}
	sock.close();
}




}
