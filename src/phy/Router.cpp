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
		header_t* header = &((packet_t*)msg)->data;
		Packet* pkt = header->payload;
		if(pkt->src.A == 0) {
			pkt->src.A = mac;
		}
		if(pkt->src.B == 0) {
			pkt->src.B = src->getMAC();
		}
		route(header, src, table.find(pkt->dst));
		route(header, src, table.find(Address(pkt->dst.A, 0)));
		if(!header->count) {
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

void Router::route(header_t* header, Node* src, Row** prow) {
	if(prow) {
		for(Node* dst : **prow) {
			if(dst && dst != src) {
				forward(header, dst);
			}
		}
	}
}

void Router::forward(header_t* header, Node* dst) {
	header->count++;
	packet_t* msg = factory.create();
	msg->data.parent = header;
	msg->data.payload = header->payload;
	msg->callback = cb_func;
	Reactor::send_async(msg, dst);
}

void Router::callback(phy::Message* msg_) {
	packet_t* msg = (packet_t*)msg_;
	header_t* parent = msg->data.parent;
	if(++(parent->acks) == parent->count) {
		msg->ack();
	}
	factory.destroy(msg);
}



}}


