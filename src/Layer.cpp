/*
 * Layer.cpp
 *
 *  Created on: Jun 19, 2016
 *      Author: mad
 */

#include "vnl/Layer.h"

namespace vnl {

Layer* layer = 0;


class Thread : public Node {
public:
	Thread() : Node("vnl/thread") {}
	
protected:
	virtual void main(Engine* engine) override {
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
	
	virtual bool handle(Message* msg) override {
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
		return Node::handle(msg);
	}
	
	virtual bool handle(Packet* pkt) override {
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
	
	void output(const String& log) {
		if(!filtering || log.to_string().find(grep) != std::string::npos) {
			std::cout << log;
		}
	}
	
private:
	bool paused = false;
	bool filtering = false;
	vnl::Queue<String> queue;
	std::string grep;
	
};


Layer::Layer() {
	assert(layer == 0);
	assert(Random64::instance == 0);
	assert(Registry::instance == 0);
	assert(Router::instance == 0);
	
	layer = this;
	domain = vnl::hash64("vnl");
	global_logs = Address(domain, vnl::hash64("global_logs"));
	
	Random64::instance = new Random64();
	Registry::instance = new Registry();
	Router::instance = new Router();
	
	vnl::spawn(new Thread());
	Registry::connect_t connect(vnl::hash64("vnl/thread"));
	send(&connect, Registry::instance);
	Registry::close_t close(connect.res);
	send(&close, Registry::instance);
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


void GlobalLogWriter::write(const String& str) {
	log_msg_t::sample_t msg;
	msg.data.node = node->mac;
	msg.data.msg = &str;
	node->send(&msg, layer->global_logs);
}



}
