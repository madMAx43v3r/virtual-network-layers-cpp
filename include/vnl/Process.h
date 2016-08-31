/*
 * Process.h
 *
 *  Created on: Jul 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_PROCESS_H_
#define INCLUDE_VNL_PROCESS_H_

#include <vnl/ProcessSupport.hxx>
#include <vnl/Map.h>
#include <vnl/Layer.h>


namespace vnl {

class Process : public ProcessBase {
public:
	Process()
		:	ProcessBase(local_domain_name, "vnl/process")
	{
	}
	
	~Process() {
		Layer::finished = true;
	}
	
protected:
	void main(vnl::Engine* engine, vnl::Message* init) {
		subscribe(local_domain_name, "vnl/announce");
		subscribe(local_domain_name, "vnl/topic");
		subscribe(local_domain_name, "vnl/log");
		subscribe(local_domain_name, "vnl/shutdown");
		subscribe(local_domain_name, "vnl/exit");
		init->ack();
		run();
		while(!objects.empty()) {
			poll(100);
		}
	}
	
	void handle(const vnl::Announce& event, const vnl::Packet& packet) {
		objects[packet.src_addr] = event.instance;
	}
	
	void handle(const vnl::Exit& event, const vnl::Packet& packet) {
		objects.erase(packet.src_addr);
	}
	
	void handle(const vnl::Topic& topic) {
		topics[Address(topic.domain, topic.name)] = topic;
	}
	
	void handle(const vnl::LogMsg& event) {
		if(!paused) {
			output(event);
		} else {
			queue.push(event);
		}
	}
	
	void handle(const vnl::Shutdown& event) {
		shutdown();
	}
	
	vnl::Array<vnl::Instance> get_objects() const {
		return objects.values();
	}
	
	vnl::Array<vnl::Topic> get_topics() const {
		return topics.values();
	}
	
	vnl::Array<vnl::String> get_class_names() const {
		vnl::Array<vnl::String> list;
		// TODO
		return list;
	}
	
	void pause_log() {
		paused = true;
	}
	
	void resume_log() {
		LogMsg log;
		while(queue.pop(log)) {
			output(log);
		}
		paused = false;
	}
	
	void set_log_filter(const vnl::String& filter) {
		filtering = !filter.empty();
		grep = filter.to_string();
	}
	
	void shutdown() {
		if(!Layer::shutdown) {
			log(INFO).out << "Shutdown activated" << vnl::endl;
			Layer::shutdown = true;
			for(Instance inst : objects.values()) {
				publish(vnl::Shutdown::create(), inst.domain, inst.topic);
			}
			exit();
		}
	}
	
	void output(const vnl::LogMsg& log) {
		if(!filtering
			|| log.msg.to_string().find(grep) != std::string::npos
			|| log.topic.to_string().find(grep) != std::string::npos)
		{
			std::cout << "[" << log.topic << "] ";
			switch(log.level) {
				case ERROR: std::cout << "ERROR: "; break;
				case WARN: std::cout << "WARNING: "; break;
				case INFO: std::cout << "INFO: "; break;
				case DEBUG: std::cout << "DEBUG: "; break;
			}
			std::cout << log.msg;
		}
	}
	
private:
	Map<Address, Instance> objects;
	Map<Address, Topic> topics;
	
	bool paused = false;
	bool filtering = false;
	Queue<LogMsg> queue;
	std::string grep;
	
};


}

#endif /* INCLUDE_VNL_PROCESS_H_ */
