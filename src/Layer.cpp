/*
 * Layer.cpp
 *
 *  Created on: Jun 19, 2016
 *      Author: mad
 */

#include <vnl/ThreadEngine.h>
#include <vnl/Layer.h>
#include <vnl/Pool.h>
#include <vnl/Process.h>

#include <iostream>
#include <mutex>
#include <dirent.h>


namespace vnl {

uint64_t local_domain = 0;
const char* local_domain_name = 0;

volatile bool Layer::shutdown = false;
volatile bool Layer::finished = false;

Map<String, String> Layer::config;

Layer::Layer(const char* domain_name, const char* config_dir) {
	assert(local_domain == 0);
	assert(global_pool == 0);
	assert(shutdown == false);
	assert(finished == false);
	assert(Router::instance == 0);
	
	local_domain_name = domain_name;
	local_domain = vnl::hash64(domain_name);
	global_pool = new GlobalPool();
	
	if(config_dir) {
		parse_config(config_dir);
	}
	
	Router::instance = new Router();
	
	vnl::spawn(new Process());
}

Layer::~Layer() {
	if(!shutdown) {
		ThreadEngine engine;
		ProcessClient proc;
		proc.set_address(local_domain, "vnl/process");
		proc.connect(&engine);
		proc.shutdown();
	}
	while(!finished) {
		usleep(10*1000);
	}
	
	delete Router::instance;
	delete Random64::instance;
	delete global_pool;
	
	Page::cleanup();
	Block::cleanup();
}


const String* Layer::get_config(String domain, String topic, String name) {
	static std::mutex mutex;
	String key = domain << ":" << topic << "->" << name;
	mutex.lock();
	String* value = config.find(key);
	mutex.unlock();
	return value;
}


void Layer::parse_config(const char* root_dir) {
	DIR* dir = opendir(root_dir);
	if(!dir) {
		std::cout << "ERROR: could not open: " << root_dir << std::endl;
		return;
	}
	char buf[1024];
	dirent* domain;
	while((domain = readdir(dir)) != 0) {
		if(domain->d_type == DT_DIR && domain->d_name[0] != '.') {
			String full = String(root_dir) << "/" << domain->d_name;
			full.to_string(buf, sizeof(buf));
			DIR* dir = opendir(buf);
			if(!dir) {
				std::cout << "WARNING: could not open: " << buf << std::endl;
				continue;
			}
			dirent* topic;
			while((topic = readdir(dir)) != 0) {
				if(topic->d_type == DT_DIR && topic->d_name[0] != '.') {
					String full = String(root_dir) << "/" << domain->d_name << "/" << topic->d_name;
					full.to_string(buf, sizeof(buf));
					DIR* dir = opendir(buf);
					if(!dir) {
						std::cout << "WARNING: could not open: " << buf << std::endl;
						continue;
					}
					dirent* name;
					while((name = readdir(dir)) != 0) {
						if(name->d_type == DT_REG && name->d_name[0] != '.') {
							String full = String(root_dir) << "/" << domain->d_name << "/" << topic->d_name << "/" << name->d_name;
							full.to_string(buf, sizeof(buf));
							FILE* file = fopen(buf, "r");
							if(!file) {
								std::cout << "WARNING: could not open: " << buf << std::endl;
								continue;
							}
							String key = String(domain->d_name) << ":" << topic->d_name << "->" << name->d_name;
							String& value = config[key];
							char buf[1024];
							while(true) {
								int count = sizeof(buf)-1;
								int num = fread(buf, 1, count, file);
								if(num > 0) {
									if(feof(file) && buf[num-1] == '\n') {
										num--;
									}
									buf[num] = 0;
									value << buf;
								} else {
									break;
								}
							}
							//std::cout << "[config] " << key << " = " << value << std::endl;
							fclose(file);
						}
					}
					closedir(dir);
				}
			}
			closedir(dir);
		}
	}
	closedir(dir);
}


void GlobalLogWriter::write(const String& str) {
	vnl::LogMsg* msg = vnl::LogMsg::create();
	msg->domain = node->my_domain;
	msg->topic = node->my_topic;
	msg->level = level;
	msg->msg = str;
	node->publish(msg, local_domain_name, "vnl/log");
}



} // vnl
