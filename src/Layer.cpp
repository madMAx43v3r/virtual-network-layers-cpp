/*
 * Layer.cpp
 *
 *  Created on: Jun 19, 2016
 *      Author: mad
 */

#include <vnl/Layer.h>

namespace vnl {

Layer* layer = 0;

Layer::Layer() {
	assert(vnl::layer == 0);
	assert(Random64::instance == 0);
	assert(Registry::instance == 0);
	assert(Router::instance == 0);
	
	vnl::layer = this;
	domain = vnl::hash64("vnl/");
	global_logs = Address(domain, vnl::hash64("global_logs"));
	
	Random64::instance = new Random64();
	Registry::instance = new Registry();
	Router::instance = new Router();
	
	vnl::spawn(new Thread("vnl/thread"));
	vnl::ping("vnl/thread");
}

void Layer::shutdown() {
	Registry::shutdown_t shutdown;
	send(&shutdown, Registry::instance);
}

Layer::~Layer() {
	shutdown();
	
	delete Router::instance;
	delete Registry::instance;
	delete Random64::instance;
	
	Page::cleanup();
	Block::cleanup();
}


void Thread::main(Engine* engine) {
	Receiver logs(this, layer->global_logs);
	run();
	poll(100*1000);
	int i = 0;
	while(true) {
		Registry::get_module_list_t list;
		send(&list, Registry::instance);
		if(list.data.size() <= 1) {
			break;
		}
		if(i % 10 == 0) {
			std::cout << "Waiting for modules to exit:" << std::endl;
			int k = 0;
			for(auto& desc : list.data) {
				std::cout << "[" << k++ << "] " << desc.name << std::endl;
			}
		}
		poll(100*1000);
		i++;
	}
}

bool Thread::handle(Message* msg) {
	if(msg->msg_id == Layer::pause_log_t::MID) {
		paused = true;
	} else if(msg->msg_id == Layer::resume_log_t::MID) {
		String log;
		while(queue.pop(log)) {
			output(log);
		}
		paused = false;
	} else if(msg->msg_id == Layer::set_log_filter_t::MID) {
		Layer::set_log_filter_t* filter = (Layer::set_log_filter_t*)msg;
		filtering = !filter->data.empty();
		grep = filter->data;
	}
	return Module::handle(msg);
}

bool Thread::handle(Packet* pkt) {
	if(pkt->dst_addr == layer->global_logs) {
		log_msg_t* log = (log_msg_t*)pkt->payload;
		if(!paused) {
			output(*log->msg);
		} else {
			queue.push(*log->msg);
		}
	}
	return false;
}

void Thread::output(const String& log) {
	if(!filtering || log.to_string().find(grep) != std::string::npos) {
		std::cout << log;
	}
}


void GlobalLogWriter::write(const String& str) {
	log_msg_t::sample_t msg;
	msg.data.node = node->get_mac();
	msg.data.msg = &str;
	node->send(&msg, vnl::layer->global_logs);
}



}
