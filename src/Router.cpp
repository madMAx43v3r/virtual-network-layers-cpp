/*
 * Router.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include "Router.h"

namespace vnl {

class Router::Worker : public Runnable {
public:
	Worker(Router* router) : router(router), queue(router), tid(0) {}
	
	void run() override {
		while(true) {
			phy::Message* msg = queue.poll();
			if(msg->mid == Switch::send_t::mid) {
				Frame& frame = ((Switch::send_t*)msg)->frame;
				if(frame.src.A == 0) {
					frame.src.A = mac;
				}
				if(frame.flags == Frame::REGISTER) {
					router->configure(frame.dst, msg->src);
				}
				if(frame.flags == Frame::UNREGISTER) {
					router->unregister(frame.dst, msg->src);
				}
				router->route(frame, msg->src ? msg->src->mac : 0);
			} else if(msg->mid == Node::receive_t::mid) {
				Frame& frame = ((Node::receive_t*)msg)->frame;
				router->route(frame, 0);
			}
			msg->ack();
		}
	}
	
	Router* router;
	phy::Stream queue;
	uint64_t tid;
	
};

Router::Router(Uplink* uplink, int N) : N(N), uplink(uplink) {
	Node::configure(Address(mac, 0));
	workers.resize(N);
	for(int i = 0; i < N; ++i) {
		Worker* worker = new Worker(this);
		worker->tid = launch(worker);
		workers[i] = worker;
	}
}

Router::~Router() {
	for(auto worker : workers) {
		cancel(worker->tid);
		delete worker;
	}
}

phy::Stream* Router::route(phy::Message* msg) {
	if(Switch::route(msg) == 0) {
		uint64_t srcmac = 0;
		if(msg->mid == Switch::send_t::mid) {
			srcmac = ((Switch::send_t*)msg)->frame.src.B;
		} else if(msg->mid == Node::receive_t::mid) {
			srcmac = ((Node::receive_t*)msg)->frame.src.B;
		}
		if(srcmac) {
			return &workers[srcmac % N]->queue;
		}
	}
	return 0;
}

void Router::route(Frame& frame, uint64_t srcmac) {
	bool unicast = frame.flags & Frame::UNICAST;
	bool anycast = frame.flags & Frame::ANYCAST;
	bool multicast = frame.flags & Frame::MULTICAST;
	if(frame.dst.A == mac) {
		if(unicast) {
			auto iter = nodes.find(frame.dst.B);
			if(iter != nodes.end()) {
				forward(frame, iter->second);
			}
		} else if(multicast && frame.dst.B == -1) {
			for(auto iter : nodes) {
				forward(frame, iter.second);
			}
		}
		return;
	}
	{
		auto iter = route128.find(frame.dst);
		if(iter != route128.end()) {
			if(multicast) {
				fw_many(frame, iter->second, srcmac);
			} else {
				fw_one(frame, iter->second, anycast);
				return;
			}
		}
	}
	{
		auto iter = route64.find(frame.dst.A);
		if(iter != route64.end()) {
			if(multicast) {
				fw_many(frame, iter->second, srcmac);
			} else {
				fw_one(frame, iter->second, anycast);
				return;
			}
		}
	}
	if(srcmac != 0) {
		send(frame);
	}
}

void Router::forward(Frame& frame, phy::Object* dst) {
	phy::Object::send(Node::receive_t(frame, this, dst));
}

void Router::fw_one(Frame& frame, std::vector<phy::Object*>& list, bool anycast) {
	int N = list.size();
	phy::Object* obj = 0;
	if(anycast) {
		while(true) {
			obj = list[std::rand() % N];
			if(obj) { break; }
		}
	} else {
		for(int i = 0; i < N; ++i) {
			obj = list[i];
			if(obj) { break; }
		}
	}
	if(obj) {
		forward(frame, obj);
	}
}

void Router::fw_many(Frame& frame, std::vector<phy::Object*>& list, uint64_t srcmac) {
	int N = list.size();
	phy::SendBuffer<Node::receive_t, 10> buf;
	for(int i = 0; i < N; ++i) {
		phy::Object* dst = list[i];
		if(dst && dst->mac != srcmac) {
			phy::Object::send(buf.put(Node::receive_t(frame, this, dst, 0, true)));
		}
	}
}

void Router::configure(Address addr, phy::Object* src) {
	auto& list = get_entry(addr);
	phy::Object** slot = 0;
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

void Router::unregister(Address addr, phy::Object* src) {
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

std::vector<phy::Object*>& Router::get_entry(const Address& addr) {
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


}
