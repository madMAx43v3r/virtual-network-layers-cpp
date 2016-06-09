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

bool Router::handle(Message* msg) {
	if(msg->msg_id == Packet::MID) {
		Packet* pkt = (Packet*)msg;
		if(pkt->src_addr.A == 0) {
			pkt->src_addr.A = mac;
		}
		Base* src = msg->src;
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
		open(((open_t*)msg)->data.second, ((open_t*)msg)->data.first);
		msg->ack();
		return true;
	} else if(msg->msg_id == close_t::MID) {
		close(((close_t*)msg)->data.second, ((close_t*)msg)->data.first);
		msg->ack();
		return true;
	}
	return false;
}

void Router::open(const Address& addr, Base* src) {
	Row*& row = table[addr];
	if(!row) {
		row = new(mem.alloc<Row>()) Row(mem);
	}
	Base** pcol = 0;
	for(Base*& col : *row) {
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

void Router::close(const Address& addr, Base* src) {
	Row* row = table[addr];
	if(row) {
		for(Base*& col : *row) {
			if(col == src) {
				col = 0;
			}
		}
	}
}

void Router::route(Packet* pkt, Base* src, Row** prow) {
	if(prow) {
		for(Base* dst : **prow) {
			if(dst && dst != src) {
				forward(pkt, dst);
			}
		}
	}
}

void Router::forward(Packet* org, Base* dst) {
	org->count++;
	Packet* msg = buffer.create<Packet>(org->pkt_id);
	msg->parent = org;
	msg->src_addr = org->src_addr;
	msg->dst_addr = org->dst_addr;
	msg->payload = org->payload;
	msg->callback = &cb_func;
	Reactor::send_async(msg, dst);
}

void Router::callback(Message* msg_) {
	Packet* msg = (Packet*)msg_;
	if(++(msg->parent->acks) == msg->parent->count) {
		msg->parent->ack();
	}
}



}


