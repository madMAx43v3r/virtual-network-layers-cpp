/*
 * TcpServer.cpp
 *
 *  Created on: Jan 12, 2016
 *      Author: MWITTAL
 */

#include "TcpServer.h"

namespace vnl {

class TcpServer::Proxy : Node {
public:
	Proxy(Uplink* uplink, io::Socket* sock)
		:	Node::Node(uplink),
			sock(sock), stream(sock)
	{
		tid_reader = launch(std::bind(&Proxy::reader, this));
	}
	
	~Proxy() {
		sock->close();
		cancel(tid_reader);
		for(auto msg : sndbuf) {
			delete msg;
		}
	}
	
	virtual bool handle(phy::Message* msg) override {
		if(Node::handle(msg)) {
			return true;
		}
		if(msg->mid == receive_t::id) {
			receive_t* packet = (receive_t*)msg;
			packet->seq = nextseq++;
			if(msg->src) {
				pending[msg->src->mac | packet->seq] = packet;
			}
			write(packet);
			return true;
		} else if(msg->mid == acksig_t::id) {
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
			if(this->ackbuf.empty()) {
				phy::Object::receive(new acksig_t(0, true));
			}
			this->ackbuf.push_back((Uplink::send_t*)msg);
		};
		stream.clear();
		ByteBuffer buf(&stream);
		while(true) {
			uint32_t mid = buf.getInt();
			if(buf.error) {
				break;
			} else if(mid == Uplink::send_t::id) {
				uint64_t srcmac = buf.getLong();
				Uplink::send_t* msg;
				if(sndbuf.empty()) {
					msg = new Uplink::send_t();
				} else {
					msg = sndbuf.back();
					sndbuf.pop_back();
				}
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
		exit();
	}
	
protected:
	io::Socket* sock;
	io::StreamBuffer stream;
	phy::taskid_t tid_reader;
	
	std::unordered_map<uint64_t, receive_t*> pending;
	std::vector<Uplink::send_t*> ackbuf;
	std::vector<Uplink::send_t*> sndbuf;
	
	int32_t nextseq = 1;
	
};


TcpServer::TcpServer(Uplink* uplink, int port)
	:	uplink(uplink), port(port)
{
	tid_acceptor = launch(std::bind(&TcpServer::acceptor, this));
}

TcpServer::~TcpServer() {
	cancel(tid_acceptor);
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
