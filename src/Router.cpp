/*
 * Router.cpp
 *
 *  Created on: May 5, 2016
 *      Author: mad
 */

#include <vnl/Router.h>
#include <vnl/Sample.h>
#include <vnl/Layer.h>

namespace vnl {

Router* Router::instance = 0;

Router::Router()
	:	current_info(0)
{
	spy_list = &table[Address()];
}

bool Router::handle(Message* msg) {
	if(msg->msg_id == Packet::MID) {
		Packet* pkt = (Packet*)msg;
		for(int i = 0; i < pkt->num_hops; ++i) {
			if(pkt->route[i] == (uint32_t)vnl_mac) {
				num_drop++;
				num_cycle++;
				msg->ack();
				return true;
			}
		}
		if(pkt->num_hops < VNL_MAX_ROUTE_LENGTH) {
			pkt->route[pkt->num_hops++] = vnl_mac;
		} else {
			num_drop++;
			pkt->ack();
			return true;
		}
		
		if(enable_topic_info && pkt->pkt_id == Sample::PID) {
			Sample* sample = (Sample*)pkt->payload;
			if(sample->header) {
				vnl::info::TopicInfo& info = topic_info[pkt->dst_addr];
				if(info.send_counter == 0) {
					info.topic = sample->header->dst_topic;
					info.first_time = vnl::currentTimeMicros();
				}
				current_info = &info;
			}
		}
		
		route(pkt, table.find(pkt->dst_addr));
		route(pkt, table.find(Address(pkt->dst_addr.domain(), (uint64_t)0)));
		route(pkt, spy_list);
		
		if(current_info) {
			current_info->publishers[pkt->proxy ? pkt->proxy : pkt->src_mac]++;
			current_info->send_counter++;
			current_info->receive_counter += pkt->count;
			current_info->last_time = vnl::currentTimeMicros();
			current_info = 0;
		}
		if(!pkt->count) {
			pkt->ack();
		}
		return true;
	} else if(msg->msg_id == open_t::MID) {
		open(((open_t*)msg)->data.second, ((open_t*)msg)->data.first);
	} else if(msg->msg_id == close_t::MID) {
		close(((close_t*)msg)->data.second, ((close_t*)msg)->data.first);
	} else if(msg->msg_id == connect_t::MID) {
		connect_t* request = (connect_t*)msg;
		Node* src = request->data;
		if(src) {
			lookup[src->get_mac()] = src;
		}
	} else if(msg->msg_id == finish_t::MID) {
		Node* src = ((finish_t*)msg)->data;
		if(src) {
			lookup.erase(src->get_mac());
		}
	} else if(msg->msg_id == get_topic_info_t::MID) {
		((get_topic_info_t*)msg)->data = topic_info.values();
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

void Router::route(Packet* pkt, Row* prow) {
	if(prow) {
		for(uint64_t& dst : *prow) {
			if(dst && dst != pkt->src_mac) {
				Basic** target = lookup.find(dst);
				if(target) {
					forward(pkt, *target);
					if(enable_topic_info && current_info) {
						current_info->subscribers[dst]++;
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
	Packet* msg = buffer.create();
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


} // vnl
