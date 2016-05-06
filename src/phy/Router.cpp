/*
 * Router.cpp
 *
 *  Created on: May 5, 2016
 *      Author: mad
 */

#include "phy/Router.h"


namespace vnl { namespace phy {

Router::Router()
	:	table(mem),
		cb_rcv(std::bind(&Router::callback_rcv, this, std::placeholders::_1))
{
}

bool Router::handle(Message* msg) {
	Node* src = msg->src;
	switch(msg->mid) {
	case connect_t::id:
		if(src) {
			Row*& row = table[((connect_t*)msg)->data];
			if(!row) {
				row = mem.create<Row>();
			}
			Node** pcol = 0;
			for(Node*& col : *row) {
				if(col == 0) {
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
		msg->ack();
		return true;
	case close_t::id:
		if(src) {
			Row* row = table[((close_t*)msg)->data];
			if(row) {
				for(Node*& col : *row) {
					if(col == src) {
						col = 0;
					}
				}
			}
		}
		msg->ack();
		return true;
	case send_t::id: {
		Packet* packet = (Packet*)((send_t*)msg)->data;
		if(!route(packet, src)) {
			msg->ack();
		}
		return true;
	}
	}
	return false;
}

int Router::route(Packet* pkt, Node* src) {
	Row** prow = table.find(pkt->dst);
	if(prow) {
		for(Node* dst : **prow) {
			if(dst != src) {
				forward(pkt, dst);
			}
		}
	}
	return pkt->count;
}

void Router::forward(Packet* pkt, Node* dst) {
	pkt->count++;
	receive_t* msg = rcvbuf.create();
	msg->data = pkt;
	msg->callback = cb_rcv;
	send_async(msg, dst);
}

void Router::callback_rcv(phy::Message* msg_) {
	receive_t* msg = (receive_t*)msg_;
	if(++(msg->data->acks) == msg->data->count) {
		msg->ack();
	}
	rcvbuf.destroy(msg);
}



}}


