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
		:	SpyToolBase(domain, "SpyTool"), running(false)
	{
	}
	
protected:
	void main(vnl::Engine* engine) {
		add_input(input);
		run();
		stop();
		input.close();
	}
	
	bool handle(Packet* pkt) {
		if(get_channel() == &input) {
			if(pkt->pkt_id == Sample::PID) {
				Sample* sample = (Sample*)pkt->payload;
				if(sample->header && !topics.find(sample->dst_addr)) {
					log(DEBUG).out << "New topic: " << sample->header->dst_topic.to_string() << vnl::endl;
					topics[sample->dst_addr] = sample->header->dst_topic;
					if(running) {
						update();
					}
				}
				if(vnl_dorun && running) {
					process(sample);
				}
			}
			return false;
		}
		return Super::handle(pkt);
	}
	
	void process(Sample* sample) {
		if(select.find(sample->dst_addr) || sample->dst_addr.topic() == filter || filter.empty()) {
			Value* value = sample->data;
			Topic* topic = topics.find(sample->dst_addr);
			if(value) {
				if(topic) {
					std::cout << vnl::currentTimeMillis() << " " << topic->domain
						<< " : " << topic->name << " -> " << value->get_type_name() << std::endl;
				} else {
					std::cout << vnl::currentTimeMillis() << " " << sample->dst_addr
						<< " -> " << value->get_type_name() << std::endl;
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
		input.subscribe(Address());
		update();
	}
	
	void stop() {
		if(running) {
			running = false;
			select.clear();
			input.unsubscribe(Address());
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
