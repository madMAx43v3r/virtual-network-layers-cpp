/*
 * Router.cpp
 *
 *  Created on: May 5, 2016
 *      Author: mad
 */

#include "vnl/Router.h"


namespace vnl {

Router* Router::instance = 0;

Router::Router()
	:	buffer(mem),
		cb_func(std::bind(&Router::callback, this, std::placeholders::_1))
{
}

bool Router::handle(phy::Message* msg) {
	phy::Node* src = msg->src;
	if(msg->msg_id == Packet::MID) {
		Packet* pkt = (Packet*)msg;
		if(pkt->src_addr.A == 0) {
			pkt->src_addr.A = mac;
		}
		if(src) {
			if(pkt->src_addr.B == 0) {
				pkt->src_addr.B = src->getMAC();
			}
		}
		route(pkt, src, table.find(pkt->dst_addr));
		route(pkt, src, table.find(Address(pkt->dst_addr.A, 0)));
		if(!pkt->count) {
			msg->ack();
		}
		return true;
	} else if(msg->msg_id == open_t::MID) {
		open(((open_t*)msg)->data, src);
		msg->ack();
		return true;
	} else if(msg->msg_id == close_t::MID) {
		close(((close_t*)msg)->data, src);
		msg->ack();
		return true;
	}
	return false;
}

void Router::open(const Address& addr, phy::Node* src) {
	Row*& row = table[addr];
	if(!row) {
		row = new(mem.alloc<Row>()) Row(mem);
	}
	phy::Node** pcol = 0;
	for(phy::Node*& col : *row) {
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

void Router::close(const Address& addr, phy::Node* src) {
	Row* row = table[addr];
	if(row) {
		for(phy::Node*& col : *row) {
			if(col == src) {
				col = 0;
			}
		}
	}
}

void Router::route(Packet* pkt, phy::Node* src, Row** prow) {
	if(prow) {
		for(phy::Node* dst : **prow) {
			if(dst && dst != src) {
				forward(pkt, dst);
			}
		}
	}
}

void Router::forward(Packet* org, phy::Node* dst) {
	org->count++;
	Packet* msg = buffer.create<Packet>(org->pkt_id);
	msg->parent = org;
	msg->src_addr = org->src_addr;
	msg->dst_addr = org->dst_addr;
	msg->payload = org->payload;
	msg->callback = &cb_func;
	Reactor::send_async(msg, dst);
}

void Router::callback(phy::Message* msg_) {
	Packet* msg = (Packet*)msg_;
	if(++(msg->parent->acks) == msg->parent->count) {
		msg->parent->ack();
	}
}



}


