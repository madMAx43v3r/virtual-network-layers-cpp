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
		subscribe(local_domain, "vnl/announce");
		subscribe(local_domain, "vnl/log");
		subscribe(local_domain, "vnl/shutdown");
		subscribe(local_domain, "vnl/exit");
		init->ack();
		run();
		while(!objects.empty()) {
			poll(100);
		}
	}
	
	void handle(const vnl::Announce& event, const vnl::Packet& packet) {
		objects[packet.src_addr] = event.instance;
	}
	
	void handle(const vnl::LogMsg& event, const vnl::Packet& packet) {
		if(!paused) {
			output(event);
		} else {
			queue.push(event);
		}
	}
	
	void handle(const vnl::Shutdown& event, const vnl::Packet& packet) {
		shutdown();
	}
	
	void handle(const vnl::Exit& event, const vnl::Packet& packet) {
		objects.erase(packet.src_addr);
	}
	
	vnl::List<vnl::String> get_domains() const {
		vnl::List<vnl::String> list;
		// TODO
		return list;
	}
	
	vnl::Array<vnl::Instance> get_objects() const {
		return objects.values();
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
			for(Address addr : objects.keys()) {
				publish(vnl::Shutdown::create(), addr);
			}
			exit();
		}
	}
	
	void output(const vnl::LogMsg& log) {
		if(!filtering || log.msg.to_string().find(grep) != std::string::npos) {
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
	
	bool paused = false;
	bool filtering = false;
	Queue<LogMsg> queue;
	std::string grep;
	
};


}

#endif /* INCLUDE_VNL_PROCESS_H_ */