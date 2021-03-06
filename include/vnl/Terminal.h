/*
 * Terminal.h
 *
 *  Created on: Jun 19, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_TERMINAL_H_
#define INCLUDE_VNL_TERMINAL_H_

#include <vnl/ProcessClient.hxx>
#include <vnl/SpyToolClient.hxx>
#include <vnl/SpyTool.h>

#include <iostream>


namespace vnl {

class Terminal : public Object {
public:
	Terminal(const String& domain = local_domain_name)
		:	Object(domain, "Terminal")
	{
	}
	
protected:
	void main(Engine* engine) {
		process.set_address(vnl::local_domain_name, "Process");
		process.connect(engine);
		std::string input;
		while(poll(0)) {
			std::getline(std::cin, input);
			if(std::cin.eof()) {
				break;
			}
			pause();
			print_help();
			std::cout << "Terminal: ";
			std::cout.flush();
			std::getline(std::cin, input);
			if(input == "quit" || input == "q") {
				resume();
				publish(vnl::Shutdown::create(), local_domain_name, "vnl.shutdown", true);
				break;
			} else if(input == "log" || input == "l") {
				std::cout << "[0] All" << std::endl;
				Array<Instance> list = process.get_objects();
				int index = 1;
				for(Instance& desc : list) {
					std::cout << "[" << index++ << "] " << desc.domain << ":" << desc.topic << std::endl;
				}
				std::cout << "Select module: ";
				std::cout.flush();
				std::getline(std::cin, input);
				int sel = atoi(input.c_str());
				std::cout << "Help: 1=error 2=warn 3=info 4=debug ..." << std::endl;
				std::cout << "Select level: ";
				std::cout.flush();
				std::getline(std::cin, input);
				int level = atoi(input.c_str());
				if(level > 0) {
					if(sel == 0) {
						log(INFO).out << "Setting log_level for all to " << level << vnl::endl;
						for(Instance& desc : list) {
							set_log_level(engine, desc, level);
						}
					} else if(sel > 0 && sel <= list.size()) {
						log(INFO).out << "Setting log_level to " << level << " for " << list[sel-1].topic << vnl::endl;
						set_log_level(engine, list[sel-1], level);
					} else {
						log(ERROR).out << "invalid input" << vnl::endl;
					}
				} else {
					log(ERROR).out << "invalid input" << vnl::endl;
				}
			} else if(input == "topics" || input == "t") {
				Router::get_topic_info_t msg;
				send(&msg, Router::instance);
				int64_t now = vnl::currentTimeMicros();
				for(vnl::info::TopicInfo& info : msg.data) {
					std::cout << "  " << info.topic.domain << " : " << info.topic.name << " | " << info.send_counter
							<< " sent | " << info.receive_counter << " received | "
							<< float(info.last_time-info.first_time)/(info.send_counter+1)/1e6 << "s cycle | "
							<< float(now-info.last_time)/1e6 << "s ago" << std::endl;
				}
				std::cout << "Help: press enter to continue" << std::endl;
				std::getline(std::cin, input);
			} else if(input.find("grep ") == 0) {
				std::cout << "Help: press enter to stop" << std::endl;
				std::string filter = input.substr(5, -1);
				process.set_log_filter(filter);
				resume();
				std::getline(std::cin, input);
				process.set_log_filter("");
				std::cout << "grep done" << std::endl;
			} else if(input.find("spy") == 0) {
				std::cout << "Help: press enter to stop" << std::endl;
				vnl::SpyToolClient tool;
				tool = vnl::spawn(new vnl::SpyTool());
				tool.connect(engine);
				if(input.size() > 4) {
					std::string filter = input.substr(4, -1);
					tool.set_filter(filter);
				} else {
					tool.set_filter("");
				}
				std::getline(std::cin, input);
				tool.exit();
			} else if(input.find("dump") == 0) {
				std::cout << "Help: press enter to stop" << std::endl;
				vnl::SpyToolClient tool;
				tool = vnl::spawn(new vnl::SpyTool());
				tool.connect(engine);
				tool.set_dump(true);
				if(input.size() > 5) {
					std::string filter = input.substr(5, -1);
					tool.set_filter(filter);
				} else {
					tool.set_filter("");
				}
				std::getline(std::cin, input);
				tool.exit();
			} else {
				print_help();
			}
			resume();
		}
	}
	
	void pause() {
		process.pause_log();
	}
	
	void resume() {
		process.resume_log();
	}
	
	void print_help() {
		std::cout << "Help: quit | log | topics | grep <expr> | spy [expr] | dump [expr]" << std::endl;
	}
	
	void set_log_level(Engine* engine, Instance& node, int level) {
		if(node.topic == my_topic) {
			return;
		}
		ObjectClient client;
		client.set_fail(true);
		client.set_timeout(100);
		client.set_address(node.domain, node.topic);
		client.connect(engine);
		try {
			client.set_vnl_log_level(level);
		} catch(...) {
			log(ERROR).out << "set_log_level() failed for " << node.topic << vnl::endl;
		}
	}
	
private:
	vnl::ProcessClient process;
	
};


} // vnl

#endif /* INCLUDE_VNL_TERMINAL_H_ */
