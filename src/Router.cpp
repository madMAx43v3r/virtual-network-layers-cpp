/*
 * Router.cpp
 *
 *  Created on: May 5, 2016
 *      Author: mad
 */

#include <vnl/Router.h>
#include <vnl/Layer.h>

namespace vnl {

Router* Router::instance = 0;

bool Router::handle(Message* msg) {
	if(msg->msg_id == Packet::MID) {
		Packet* pkt = (Packet*)msg;
		Basic* src = msg->src;
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
			pkt->ack();
			return true;
		}
		route(pkt, src, table.find(pkt->dst_addr));
		route(pkt, src, table.find(Address(pkt->dst_addr.domain(), (uint64_t)0)));
		if(!pkt->count) {
			pkt->ack();
		}
		return true;
	} else if(msg->msg_id == open_t::MID) {
		open(((open_t*)msg)->data.second, ((open_t*)msg)->data.first);
	} else if(msg->msg_id == close_t::MID) {
		close(((close_t*)msg)->data.second, ((close_t*)msg)->data.first);
	} else if(msg->msg_id == Pipe::connect_t::MID) {
		Basic* src = ((Pipe::connect_t*)msg)->args;
		if(src) {
			lookup[src->get_mac()] = src;
			((Pipe::connect_t*)msg)->res = true;
		}
	} else if(msg->msg_id == Pipe::close_t::MID) {
		Basic* src = ((Pipe::close_t*)msg)->data;
		if(src) {
			lookup.erase(src->get_mac());
		}
	}
	return false;
}

void Router::open(const Address& addr, uint64_t src) {
	Row& row = table[addr];
	uint64_t* pcol = 0;
	for(uint64_t& col : row) {
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

void Router::close(const Address& addr, uint64_t src) {
	Row* row = table.find(addr);
	if(row) {
		for(uint64_t& col : *row) {
			if(col == src) {
				col = 0;
			}
		}
	}
}

void Router::route(Packet* pkt, Basic* src, Row* prow) {
	if(prow) {
		for(uint64_t& dst : *prow) {
			if(dst) {
				Basic** target = lookup.find(dst);
				if(target) {
					if(*target != src) {
						forward(pkt, *target);
					}
				} else {
					dst = 0;
				}
			}
		}
	}
}

void Router::forward(Packet* org, Basic* dst) {
	org->count++;
	Packet* msg = buffer.create<Packet>();
	msg->dst = dst;
	msg->copy_from(org);
	Reactor::send_async(msg, dst);
}

void Router::callback(Message* msg) {
	if(msg->msg_id == Packet::MID) {
		Packet* pkt = (Packet*)msg;
		if(++(pkt->parent->acks) == pkt->parent->count) {
			pkt->parent->ack();
		}
	}
}



}


