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
	:	cb_func(std::bind(&Router::callback, this, std::placeholders::_1))
{
}

bool Router::handle(Message* msg) {
	if(msg->msg_id == Packet::MID) {
		Packet* pkt = (Packet*)msg;
		if(pkt->src_addr.A == 0) {
			pkt->src_addr.A = mac;
		}
		Basic* src = msg->src;
		if(src) {
			if(pkt->src_addr.B == 0) {
				pkt->src_addr.B = src->get_mac();
			}
		}
		for(int i = 0; i < pkt->num_hops; ++i) {
			if(pkt->route[i] == (uint32_t)mac) {
				num_drop++;
				num_cycle++;
				msg->ack();
				return true;
			}
		}
		if(pkt->num_hops < VNL_MAX_ROUTE_LENGTH) {
			pkt->route[pkt->num_hops++] = mac;
		} else {
			num_drop++;
			msg->ack();
			return true;
		}
		// direct match
		route(pkt, src, table.find(pkt->dst_addr));
		// domain match
		uint64_t domain = pkt->dst_addr.A;
		Row* prow = table.find(Address(domain, (uint64_t)0));
		if(prow == 0) {
			domains.push_back(domain);
			table[Address(domain, (uint64_t)0)] = Row();
		}
		route(pkt, src, prow);
		if(!pkt->count) {
			msg->ack();
		}
		return true;
	} else if(msg->msg_id == open_t::MID) {
		open(((open_t*)msg)->data.second, ((open_t*)msg)->data.first);
	} else if(msg->msg_id == close_t::MID) {
		close(((close_t*)msg)->data.second, ((close_t*)msg)->data.first);
	} else if(msg->msg_id == get_domain_list_t::MID) {
		((get_domain_list_t*)msg)->data = domains;
	}
	return false;
}

void Router::open(const Address& addr, Basic* src) {
	Row& row = table[addr];
	Basic** pcol = 0;
	for(Basic*& col : row) {
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
		row.push_back(src);
	}
}

void Router::close(const Address& addr, Basic* src) {
	Row* row = table.find(addr);
	if(row) {
		for(Basic*& col : *row) {
			if(col == src) {
				col = 0;
			}
		}
	}
}

void Router::route(Packet* pkt, Basic* src, Row* prow) {
	if(prow) {
		for(Basic* dst : *prow) {
			if(dst && dst != src) {
				forward(pkt, dst);
			}
		}
	}
}

void Router::forward(Packet* org, Basic* dst) {
	org->count++;
	Packet* msg = buffer.create<Packet>();
	msg->pkt_id = org->pkt_id;
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


