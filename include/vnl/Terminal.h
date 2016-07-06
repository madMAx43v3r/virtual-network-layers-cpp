/*
 * Terminal.h
 *
 *  Created on: Jun 19, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_TERMINAL_H_
#define INCLUDE_VNL_TERMINAL_H_

#include <iostream>

#include "vnl/Module.h"
#include "vnl/Layer.h"


namespace vnl {

class Terminal : public Module {
public:
	Terminal() : Module("vnl/terminal") {}
	
protected:
	virtual void main(Engine* engine) override {
		Reference<Module> writer_ref(engine, "vnl/thread");
		writer = writer_ref.get();
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
				break;
			} else if(input == "log") {
				std::cout << "[0] All" << std::endl;
				Registry::get_module_list_t list;
				send(&list, Registry::instance);
				int index = 1;
				for(auto& desc : list.data) {
					std::cout << "[" << index++ << "] " << desc.name << std::endl;
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
						for(auto& desc : list.data) {
							set_log_level(engine, desc.mac, level);
						}
					} else if(sel > 0 && sel <= list.data.size()) {
						log(INFO).out << "Setting log_level to " << vnl::dec(level) << " for " << list.data[sel-1].name << vnl::endl;
						set_log_level(engine, list.data[sel-1].mac, level);
					} else {
						log(ERROR).out << "invalid input" << vnl::endl;
					}
				} else {
					log(ERROR).out << "invalid input" << vnl::endl;
				}
			} else if(input.find("grep ") == 0) {
				std::cout << "Help: press enter to stop grep" << std::endl;
				std::string filter = input.substr(5, -1);
				Layer::set_log_filter_t msg(filter);
				send(&msg, writer);
				resume();
				std::getline(std::cin, input);
				Layer::set_log_filter_t reset("");
				send(&reset, writer);
				log(INFO).out << "grep done" << vnl::endl;
			} else {
				print_help();
			}
			resume();
		}
	}
	
	void pause() {
		Layer::pause_log_t pause;
		send(&pause, writer);
	}
	
	void resume() {
		Layer::resume_log_t resume;
		send(&resume, writer);
	}
	
	void print_help() {
		std::cout << "Help: quit | log | grep <filter>" << std::endl;
	}
	
	void set_log_level(Engine* engine, uint64_t node, int level) {
		Reference<Module> ref(engine, node);
		Module* dst = ref.try_get();
		if(dst) {
			Module::set_log_level_t msg(level);
			send(&msg, dst);
		}
	}
	
private:
	Module* writer = 0;
	
};






}

#endif /* INCLUDE_VNL_TERMINAL_H_ */
