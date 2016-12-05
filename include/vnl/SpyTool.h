/*
 * SpyTool.h
 *
 *  Created on: Aug 26, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_SPYTOOL_H_
#define INCLUDE_VNL_SPYTOOL_H_

#include <vnl/Sample.h>
#include <vnl/SpyToolSupport.hxx>

#include <iostream>
#include <string>


namespace vnl {

class SpyTool : public SpyToolBase {
public:
	SpyTool(const String& domain = local_domain_name)
		:	SpyToolBase(domain, "vnl.SpyTool"), running(false)
	{
	}
	
protected:
	void main(vnl::Engine* engine) {
		input.connect(engine, 0);
		input.listen(this);
		Router::hook_t enable(vnl::make_pair<Basic*>(&input, true));
		send(&enable, Router::instance);
		run();
		stop();
	}
	
	bool handle(Stream::notify_t* msg) {
		if(msg->data == &input) {
			Message* in;
			while(input.pop(in)) {
				if(in->msg_id == Packet::MID) {
					Packet* pkt = (Packet*)in;
					if(pkt->pkt_id == Sample::PID) {
						Sample* sample = (Sample*)pkt->payload;
						if(vnl_dorun && sample) {
							process(sample, pkt);
						}
					}
				}
				in->ack();
			}
			return false;
		}
		return Super::handle(msg);
	}
	
	void process(Sample* sample, Packet* pkt) {
		if(sample->header && !topics.find(pkt->dst_addr)) {
			log(DEBUG).out << "New topic: " << sample->header->dst_topic.to_string() << vnl::endl;
			topics[pkt->dst_addr] = sample->header->dst_topic;
			if(running) {
				update();
			}
		}
		if(select.find(pkt->dst_addr) || pkt->dst_addr.topic() == filter || filter.empty()) {
			Value* value = sample->data;
			Topic* topic = topics.find(pkt->dst_addr);
			if(running && value) {
				if(topic) {
					std::cout << vnl::currentTimeMillis() << " " << topic->domain
						<< " : " << topic->name << " -> " << value->type_name() << std::endl;
				} else {
					std::cout << vnl::currentTimeMillis() << " " << pkt->dst_addr
						<< " -> " << value->type_name() << std::endl;
				}
				if(dump) {
					std::cout << value->to_string() << std::endl;
				}
			}
		}
	}
	
	void update() {
		if(filter.empty()) {
			for(Address& addr : topics.keys()) {
				select[addr] = true;
			}
		} else {
			std::string expr = filter.to_string();
			for(auto& entry : topics.entries()) {
				Address& addr = entry.first;
				Topic& topic = entry.second;
				if(!select.find(addr)) {
					if(topic.name.to_string().find(expr) != std::string::npos) {
						select[addr] = true;
						std::cout << "Matched " << topic.domain << " " << topic.name << std::endl;
					}
				}
			}
		}
	}
	
	void set_filter(const vnl::String& expr) {
		filter = expr;
		running = true;
		update();
	}
	
	void stop() {
		if(running) {
			running = false;
			select.clear();
			Router::hook_t disable(vnl::make_pair<Basic*>(&input, false));
			send(&disable, Router::instance);
		}
	}
	
private:
	bool running;
	String filter;
	Stream input;
	
	Map<Address, Topic> topics;
	Map<Address, bool> select;
	
};



} // vnl

#endif /* INCLUDE_VNL_SPYTOOL_H_ */
