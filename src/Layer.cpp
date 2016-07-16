/*
 * Layer.cpp
 *
 *  Created on: Jun 19, 2016
 *      Author: mad
 */

#include <vnl/Layer.h>
#include <vnl/Process.h>


namespace vnl {

uint64_t local_domain = 0;
const char* local_domain_name = 0;

GlobalPool* global_pool = 0;

Layer::Layer(const char* domain_name) {
	assert(local_domain == 0);
	assert(global_pool == 0);
	assert(Random64::instance == 0);
	assert(Router::instance == 0);
	
	local_domain_name = domain_name;
	local_domain = vnl::hash64(domain_name);
	global_pool = new GlobalPool();
	
	Random64::instance = new Random64();
	Router::instance = new Router();
	
	vnl::spawn(new Process());
}

Layer::~Layer() {
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
