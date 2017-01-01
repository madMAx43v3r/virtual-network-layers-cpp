/*
 * Process.h
 *
 *  Created on: Jul 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_PROCESS_H_
#define INCLUDE_VNL_PROCESS_H_

#include <vnl/Map.h>
#include <vnl/Layer.h>

#include <vnl/ProcessSupport.hxx>
#include <vnl/info/TopicInfoList.hxx>


namespace vnl {

class Process : public ProcessBase {
public:
	Process()
		:	ProcessBase(vnl::local_domain_name, "Process")
	{
		name = local_domain_name;
	}
	
protected:
	void main(vnl::Engine* engine, vnl::Message* init) {
		subscribe(local_domain_name, "vnl.announce");
		subscribe(local_domain_name, "vnl.log");
		subscribe(local_domain_name, "vnl.shutdown");
		subscribe(local_domain_name, "vnl.exit");
		set_timeout(1000*1000*1, std::bind(&Process::update, this), VNL_TIMER_REPEAT);
		if(do_print_stats) {
			set_timeout(1000*1000*10, std::bind(&Process::print_stats, this), VNL_TIMER_REPEAT);
		}
		init->ack();
		run();
		set_timeout(1000*1000*3, std::bind(&Process::print_waitlist, this), VNL_TIMER_REPEAT);
		while(!objects.empty()) {
			poll(-1);
		}
	}
	
	void print_stats() {
		if(!paused) {
			std::cout << "[" << my_topic << "] System: "
				<< Page::num_used << "/" << Page::num_alloc << " Pages, "
				<< Block::num_used << "/" << Block::num_alloc << " Blocks, "
				<< Layer::num_threads << " Threads" << std::endl;
		}
	}
	
	void print_waitlist() {
		for(Instance& obj : objects.values()) {
			std::cout << "[" << my_topic << "] Waiting on " << obj.domain << ":" << obj.topic << std::endl;
		}
	}
	
	void handle(const vnl::Announce& event, const vnl::Packet& packet) {
		objects[packet.src_addr] = event.instance;
	}
	
	void handle(const vnl::Exit& event, const vnl::Packet& packet) {
		objects.erase(packet.src_addr);
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
	
	vnl::Array<vnl::String> get_class_names() const {
		return vnl::get_class_names();
	}
	
	vnl::Map<vnl::Hash32, vnl::info::Type> get_type_info() const {
		return vnl::get_type_info();
	}
	
	vnl::info::TopicInfoList get_topic_info() const {
		return topic_info;
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
		log(INFO).out << "Received shutdown signal." << vnl::endl;
		for(Instance inst : objects.values()) {
			publish(vnl::Shutdown::create(), inst.domain, inst.topic);
		}
		exit();
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
	
	void update() {
		Router::get_topic_info_t msg;
		send(&msg, Router::instance);
		topic_info.time = vnl::currentTimeMicros();
		topic_info.topics = msg.data;
		publish(topic_info.clone(), my_private_domain, "topic_info");
	}
	
private:
	Map<Address, Instance> objects;
	vnl::info::TopicInfoList topic_info;
	
	bool paused = false;
	bool filtering = false;
	Queue<LogMsg> queue;
	std::string grep;
	
};


}

#endif /* INCLUDE_VNL_PROCESS_H_ */
