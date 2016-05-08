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
	:	table(&mem),
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
		Packet* pkt = ((packet_t*)msg)->data;
		route(pkt, src, table.find(pkt->dst));
		route(pkt, src, table.find(Address(pkt->dst.A, 0)));
		if(!pkt->count) {
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
		row = mem.create<Row>();
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

void Router::route(Packet* pkt, Node* src, Row** prow) {
	if(prow) {
		for(Node* dst : **prow) {
			if(dst && dst != src) {
				forward(pkt, dst);
			}
		}
	}
}

void Router::forward(Packet* pkt, Node* dst) {
	pkt->count++;
	packet_t* msg = factory.create();
	msg->data = pkt;
	msg->callback = cb_func;
	send_async(msg, dst);
}

void Router::callback(phy::Message* msg_) {
	packet_t* msg = (packet_t*)msg_;
	if(++(msg->data->acks) == msg->data->count) {
		msg->ack();
	}
	factory.destroy(msg);
}



}}


