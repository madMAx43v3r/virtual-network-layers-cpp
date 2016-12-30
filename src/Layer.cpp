/*
 * Layer.cpp
 *
 *  Created on: Jun 19, 2016
 *      Author: mad
 */

#include <vnl/ThreadEngine.h>
#include <vnl/Layer.h>
#include <vnl/Random.h>
#include <vnl/Pool.h>
#include <vnl/Pipe.h>
#include <vnl/Process.h>
#include <vnl/Type.hxx>
#include <vnl/ProcessClient.hxx>

#include <iostream>
#include <mutex>
#include <dirent.h>


namespace vnl {

const char* local_domain_name = 0;
const char* local_config_name = "";


namespace internal {
	
	Map<Hash32, vnl::info::Type>* type_info_ = 0;
	
} // internal


Random64* Random64::instance = 0;

volatile bool Layer::have_shutdown = false;
std::atomic<int> Layer::num_threads(0);


const vnl::info::Type* get_type_info(Hash32 type_name) {
	static std::mutex mutex;
	std::lock_guard<std::mutex> lock(mutex);
	return internal::type_info_->find(type_name);
}


Layer::Layer(const char* domain_name, const char* config_dir)
	:	closed(false)
{
	assert(Random64::instance == 0);
	assert(Router::instance == 0);
	assert(Pipe::pool == 0);
	assert(internal::global_pool_ == 0);
	assert(internal::config_ == 0);
	assert(internal::type_info_ == 0);
	assert(have_shutdown == false);
	assert(num_threads == 0);
	
	Random64::instance = new Random64();
	Pipe::pool = new Pool<Pipe>();
	local_domain_name = domain_name;
	internal::global_pool_ = new GlobalPool();
	internal::config_ = new Map<String, String>();
	internal::type_info_ = new Map<Hash32, vnl::info::Type>(vnl::get_type_info());
	
	if(config_dir) {
		local_config_name = config_dir;
		parse_config(config_dir);
	}
	
	Router::instance = new Router();
	
	vnl::spawn(new Process());
}

Layer::~Layer() {
	close();
}

void Layer::shutdown() {
	if(!have_shutdown) {
		ThreadEngine engine;
		ProcessClient proc = Address("vnl", "Process");
		proc.set_fail(true);
		proc.connect(&engine);
		try {
			proc.shutdown();
		} catch(...) {}
		engine.flush();
		have_shutdown = true;
	}
}

void Layer::close() {
	if(closed) {
		return;
	}
	while(num_threads.load() > 0) {
		usleep(10*1000);
	}
	
	if(Pipe::get_num_open() > 0) {
		std::cout << "WARNING: " << Pipe::get_num_open() << " pipes left open at exit!" << std::endl;
	}
	
	delete Router::instance;
	delete Pipe::pool;
	delete Random64::instance;
	delete internal::global_pool_;
	delete internal::config_;
	delete internal::type_info_;
	
	Page::cleanup();
	Block::cleanup();
	closed = true;
}

void Layer::parse_config(const char* root_dir) {
	char buf[4096];
	// try to find parent
	{
		(String(root_dir) << "/parent").to_string(buf, sizeof(buf));
		FILE* parent = ::fopen(buf, "r");
		if(parent) {
			int num = ::fread(buf, 1, sizeof(buf)-1, parent);
			if(num > 1) {
				buf[num-1] = 0;
				(String(root_dir) << "/" << buf).to_string(buf, sizeof(buf));
				parse_config(buf);
			}
			::fclose(parent);
		}
	}
	// parse the whole tree
	DIR* dir = ::opendir(root_dir);
	if(!dir) {
		std::cout << "ERROR: could not open: " << root_dir << std::endl;
		return;
	}
	dirent* domain;
	while((domain = ::readdir(dir)) != 0) {
		if(domain->d_type == DT_DIR && domain->d_name[0] != '.') {
			(String(root_dir) << "/" << domain->d_name).to_string(buf, sizeof(buf));
			DIR* dir = ::opendir(buf);
			if(!dir) {
				std::cout << "WARNING: could not open: " << buf << std::endl;
				continue;
			}
			dirent* topic;
			while((topic = ::readdir(dir)) != 0) {
				if(topic->d_type == DT_DIR && topic->d_name[0] != '.') {
					(String(root_dir) << "/" << domain->d_name << "/" << topic->d_name).to_string(buf, sizeof(buf));
					DIR* dir = ::opendir(buf);
					if(!dir) {
						std::cout << "WARNING: could not open: " << buf << std::endl;
						continue;
					}
					dirent* name;
					while((name = ::readdir(dir)) != 0) {
						if(name->d_type == DT_REG && name->d_name[0] != '.') {
							(String(root_dir) << "/" << domain->d_name << "/" << topic->d_name << "/" << name->d_name).to_string(buf, sizeof(buf));
							FILE* file = ::fopen(buf, "r");
							if(!file) {
								std::cout << "WARNING: could not open: " << buf << std::endl;
								continue;
							}
							String key = String(domain->d_name) << ":" << topic->d_name << "->" << name->d_name;
							String& value = (*internal::config_)[key];
							value.clear();
							while(true) {
								int count = sizeof(buf)-1;
								int num = ::fread(buf, 1, count, file);
								if(num > 0) {
									if(::feof(file) && buf[num-1] == '\n') {
										num--;
									}
									buf[num] = 0;
									value << buf;
								} else {
									break;
								}
							}
							//std::cout << "[config] " << key << " = " << value << std::endl;
							::fclose(file);
						}
					}
					::closedir(dir);
				}
			}
			::closedir(dir);
		}
	}
	::closedir(dir);
}


void GlobalLogWriter::write(const String& str) {
	vnl::LogMsg* msg = vnl::LogMsg::create();
	msg->domain = node->my_domain;
	msg->topic = node->my_topic;
	msg->level = level;
	msg->msg = str;
	node->publish(msg, local_domain_name, "vnl.log");
}



} // vnl
