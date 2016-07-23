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


namespace vnl {

uint64_t local_domain = 0;
const char* local_domain_name = 0;

volatile bool Layer::shutdown = false;
volatile bool Layer::finished = false;

Layer::Layer(const char* domain_name) {
	assert(local_domain == 0);
	assert(global_pool == 0);
	assert(Router::instance == 0);
	
	local_domain_name = domain_name;
	local_domain = vnl::hash64(domain_name);
	global_pool = new GlobalPool();
	
	if(!Random64::instance) {
		Random64::instance = new Random64();
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


void GlobalLogWriter::write(const String& str) {
	vnl::LogMsg* msg = vnl::LogMsg::create();
	msg->domain = node->my_domain;
	msg->topic = node->my_topic;
	msg->level = level;
	msg->msg = str;
	node->publish(msg, local_domain, "vnl/log");
}



}
