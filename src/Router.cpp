/*
 * Router.cpp
 *
 *  Created on: Feb 29, 2016
 *      Author: MWITTAL
 */

#include "Router.h"

namespace vnl {

Router::Router(Uplink* uplink) : uplink(uplink) {
	Node::configure(Address(mac, 0));
}

Router::~Router() {
	
}

bool Router::handle(phy::Message* msg) {
	if(Uplink::handle(msg)) {
		return true;
	}
	Node* node = (Node*)msg->src;
	uint64_t srcmac = node ? node->mac : 0;
	switch(msg->mid) {
	case connect_t::id:
		if(node) {
			nodes[srcmac] = node;
		}
		msg->ack();
		return true;
	case disconnect_t::id:
		if(node) {
			nodes.erase(srcmac);
		}
		msg->ack();
		return true;
	case send_t::id: {
		Packet* packet = (Packet*)msg;
		Frame& frame = packet->frame;
		if(frame.src.A == 0) {
			frame.src.A = mac;
		}
		if(frame.src.B == 0) {
			frame.src.B = srcmac;
		}
		if(frame.flags == Frame::REGISTER) {
			configure(frame.dst, node);
		}
		if(frame.flags == Frame::UNREGISTER) {
			unregister(frame.dst, node);
		}
		route(packet, srcmac);
		return true;
	}
	case receive_t::id: {
		Packet* packet = (Packet*)msg;
		route(packet, 0);
		return true;
	}
	}
	return false;
}

void Router::route(Packet* msg, uint64_t srcmac) {
	Frame& frame = msg->frame;
	bool unicast = frame.flags & Frame::UNICAST;
	bool anycast = frame.flags & Frame::ANYCAST;
	bool multicast = frame.flags & Frame::MULTICAST;
	if(frame.dst.A == mac) {
		if(unicast) {
			auto iter = nodes.find(frame.dst.B);
			if(iter != nodes.end()) {
				forward(msg, iter->second);
			}
		} else if(multicast && frame.dst.B == -1) {
			for(auto iter : nodes) {
				forward(msg, iter.second);
			}
		}
		return;
	}
	{
		auto iter = route128.find(frame.dst);
		if(iter != route128.end()) {
			if(multicast) {
				fw_many(msg, iter->second, srcmac);
			} else {
				fw_one(msg, iter->second, anycast);
				return;
			}
		}
	}
	{
		auto iter = route64.find(frame.dst.A);
		if(iter != route64.end()) {
			if(multicast) {
				fw_many(msg, iter->second, srcmac);
			} else {
				fw_one(msg, iter->second, anycast);
				return;
			}
		}
	}
	if(srcmac != 0) {
		send(frame);
	}
	// Check if frame dropped
	if() {
		
	}
}

void Router::forward(Packet* msg, Node* dst) {
	phy::Object::send(Node::receive_t(msg->frame), dst);
}

void Router::fw_one(Packet* msg, std::vector<Node*>& list, bool anycast) {
	int N = list.size();
	Node* dst = 0;
	if(anycast) {
		while(true) {
			dst = list[std::rand() % N];
			if(dst) { break; }
		}
	} else {
		for(int i = 0; i < N; ++i) {
			dst = list[i];
			if(dst) { break; }
		}
	}
	if(dst) {
		forward(msg, dst);
	}
}

void Router::fw_many(Packet* msg, std::vector<Node*>& list, uint64_t srcmac) {
	int N = list.size();
	for(int i = 0; i < N; ++i) {
		Node* dst = list[i];
		if(dst && dst->mac != srcmac) {
			forward(msg, dst);
		}
	}
}

void Router::configure(Address addr, Node* src) {
	auto& list = get_entry(addr);
	Node** slot = 0;
	for(auto& obj : list) {
		if(obj == src) { return; }
		if(obj == 0) { slot = &obj; }
	}
	if(slot) {
		*slot = src;
	} else {
		list.push_back(src);
	}
}

void Router::unregister(Address addr, Node* src) {
	auto& list = get_entry(addr);
	int count = 0;
	for(auto& obj : list) {
		if(obj == src) { obj = 0; }
		if(obj == 0) { count++; }
	}
	if(count == list.size()) {
		clear_entry(addr);
	}
}

std::vector<Node*>& Router::get_entry(const Address& addr) {
	if(addr.B == 0) {
		return route64[addr.A];
	} else {
		return route128[addr];
	}
}

void Router::clear_entry(const Address& addr) {
	if(addr.B == 0) {
		route64.erase(addr.A);
	} else {
		route128.erase(addr);
	}
}

Node* Router::get_node(uint64_t mac) {
	auto iter = nodes.find(mac);
	if(iter != nodes.end()) {
		return iter->second;
	}
	return 0;
}


}
