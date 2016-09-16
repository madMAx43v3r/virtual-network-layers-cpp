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
#include <vnl/ProcessClient.hxx>

#include <iostream>
#include <string>


namespace vnl {

class SpyTool : public SpyToolBase {
public:
	SpyTool(const String& domain = local_domain_name)
		:	SpyToolBase(domain, "vnl/spytool")
	{
		running = false;
	}
	
protected:
	void main(vnl::Engine* engine) {
		subscribe(my_domain, "vnl/topic");
		
		vnl::ProcessClient process;
		process.set_address(my_domain, "vnl/process");
		process.connect(engine);
		vnl::Array<Topic> list;
		if(process.get_topics(list) == VNL_SUCCESS) {
			log(DEBUG).out << "process.get_topics() returned " << topics.size() << " topics" << vnl::endl;
			for(Topic& topic : list) {
				topics[Address(topic.domain, topic.name)] = topic;
			}
		} else {
			log(ERROR).out << "process.get_topics() failed" << vnl::endl;
		}
		
		run();
	}
	
	bool handle(vnl::Packet* pkt) {
		if(running && pkt->pkt_id == vnl::Sample::PID) {
			Sample* sample = (Sample*)pkt->payload;
			if(sample && select.find(pkt->dst_addr)) {
				Value* value = sample->data;
				Topic* topic = topics.find(pkt->dst_addr);
				if(topic && value) {
					std::cout << vnl::currentTimeMillis() << " " << topic->domain
							<< " : " << topic->name << " -> " << value->type_name() << std::endl;
					if(dump) {
						std::cout << value->to_string() << std::endl;
					}
				}
			}
		}
		return Super::handle(pkt);
	}
	
	void handle(const vnl::Topic& topic) {
		log(DEBUG).out << "New topic: " << topic.to_string() << vnl::endl;
		Address addr(topic.domain, topic.name);
		if(!topics.find(addr)) {
			topics[addr] = topic;
			if(running) {
				update();
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
		for(Address& addr : select.keys()) {
			subscribe(addr);
		}
	}
	
	void set_filter(const vnl::String& expr) {
		filter = expr;
		running = true;
		update();
	}
	
	void stop() {
		running = false;
		for(Address& addr : select.keys()) {
			unsubscribe(addr);
		}
		select.clear();
	}
	
private:
	bool running;
	String filter;
	
	Map<Address, Topic> topics;
	Map<Address, bool> select;
	
};



}

#endif /* INCLUDE_VNL_SPYTOOL_H_ */
