/*
 * Terminal.h
 *
 *  Created on: Jun 19, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_TERMINAL_H_
#define INCLUDE_VNL_TERMINAL_H_

#include <iostream>

#include <vnl/ProcessSupport.hxx>
#include <vnl/Object.h>
#include <vnl/Layer.h>


namespace vnl {

class Terminal : public Object {
public:
	Terminal() : Object(local_domain_name, "vnl/terminal") {}
	
protected:
	void main(Engine* engine, Message* init) {
		init->ack();
		process.set_address(local_domain, "vnl/process");
		process.connect(engine);
		std::string input;
		while(poll(0)) {
			std::getline(std::cin, input);
			pause();
			print_help();
			std::cout << "Terminal: ";
			std::cout.flush();
			std::getline(std::cin, input);
			if(input == "quit") {
				resume();
				publish(vnl::Shutdown::create(), local_domain_name, "vnl/shutdown");
				break;
			} else if(input == "log") {
				std::cout << "[0] All" << std::endl;
				Array<Instance> list;
				process.get_objects(list);
				int index = 1;
				for(auto& desc : list) {
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
						log(INFO).out << "Setting log_level for all to " << vnl::dec(level) << vnl::endl;
						for(auto& desc : list) {
							set_log_level(engine, desc, level);
						}
					} else if(sel > 0 && sel <= list.size()) {
						log(INFO).out << "Setting log_level to " << vnl::dec(level) << " for " << list[sel-1].topic << vnl::endl;
						set_log_level(engine, list[sel-1], level);
					} else {
						log(ERROR).out << "invalid input" << vnl::endl;
					}
				} else {
					log(ERROR).out << "invalid input" << vnl::endl;
				}
			} else if(input.find("grep ") == 0) {
				std::cout << "Help: press enter to stop grep" << std::endl;
				std::string filter = input.substr(5, -1);
				process.set_log_filter(filter);
				resume();
				std::getline(std::cin, input);
				process.set_log_filter("");
				log(INFO).out << "grep done" << vnl::endl;
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
		std::cout << "Help: quit | log | grep <filter>" << std::endl;
	}
	
	void set_log_level(Engine* engine, Instance& node, int level) {
		if(node.topic == my_topic) {
			return;
		}
		ObjectClient client;
		client.set_address(node.domain, node.topic);
		client.connect(engine);
		client.set_log_level(level);
	}
	
private:
	vnl::ProcessClient process;
	
};






}

#endif /* INCLUDE_VNL_TERMINAL_H_ */
