/*
 * Router.cpp
 *
 *  Created on: May 5, 2016
 *      Author: mad
 */

#include "phy/Router.h"


namespace vnl { namespace phy {

Router* Router::instance = 0;

Router::Router()
	:	factory(mem),
		cb_func(std::bind(&Router::callback, this, std::placeholders::_1))
{
}

bool Router::handle(Message* msg) {
	Node* src = msg->src;
	switch(msg->mid) {
	case connect_t::id:
		if(src) {
			connect(((connect_t*)msg)->data, src);
		}
		msg->ack();
		return true;
	case close_t::id:
		if(src) {
			close(((close_t*)msg)->data, src);
		}
		msg->ack();
		return true;
	case packet_t::id: {
		packet_t* packet = (packet_t*)msg;
		Packet* pkt = packet->data.packet;
		if(pkt->src.A == 0) {
			pkt->src.A = mac;
		}
		if(pkt->src.B == 0) {
			pkt->src.B = src->getMAC();
		}
		route(packet, src, table.find(pkt->dst));
		route(packet, src, table.find(Address(pkt->dst.A, 0)));
		if(!packet->data.count) {
			msg->ack();
		}
		return true;
	}
	}
	return false;
}

void Router::connect(const Address& addr, Node* src) {
	Row*& row = table[addr];
	if(!row) {
		row = new(mem.alloc<Row>()) Row(mem);
	}
	Node** pcol = 0;
	for(Node*& col : *row) {
		if(col == 0) {
			pcol = &col;
		} else if(col == src) {
			pcol = &col;
			break;
		}
	}
	if(pcol) {
		*pcol = src;
	} else {
		row->push_back(src);
	}
}

void Router::close(const Address& addr, Node* src) {
	Row* row = table[addr];
	if(row) {
		for(Node*& col : *row) {
			if(col == src) {
				col = 0;
			}
		}
	}
}

void Router::route(packet_t* packet, Node* src, Row** prow) {
	if(prow) {
		for(Node* dst : **prow) {
			if(dst && dst != src) {
				forward(packet, dst);
			}
		}
	}
}

void Router::forward(packet_t* org, Node* dst) {
	org->data.count++;
	packet_t* msg = factory.create();
	msg->data.parent = org;
	msg->data.packet = org->data.packet;
	msg->callback = &cb_func;
	Reactor::send_async(msg, dst);
}

void Router::callback(phy::Message* msg_) {
	packet_t* msg = (packet_t*)msg_;
	packet_t* parent = (packet_t*)msg->data.parent;
	if(++(parent->data.acks) == parent->data.count) {
		parent->ack();
	}
	factory.destroy(msg);
}



}}


