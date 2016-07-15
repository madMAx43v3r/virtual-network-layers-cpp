/*
 * test_router.cpp
 *
 *  Created on: May 8, 2016
 *      Author: mad
 */

#include "vnl/ThreadEngine.h"
#include "vnl/FiberEngine.h"
#include "vnl/Layer.h"
#include "vnl/Router.h"
#include "vnl/Terminal.h"

#include "../src/CRC64.cpp"
#include "../src/Engine.cpp"
#include "../src/Message.cpp"
#include "../src/Random.cpp"
#include "../src/Module.cpp"
#include "../src/Registry.cpp"
#include "../src/Router.cpp"
#include "../src/Layer.cpp"
#include "../src/FiberEngine.cpp"


struct test_msg_t {
	vnl::String text;
	VNL_SAMPLE(test_msg_t);
};

vnl::Address address("domain", "topic");

class Consumer : public vnl::Module {
protected:
	virtual void main(vnl::Engine* engine) {
		//vnl::Util::stick_to_core(mac % 3 + 1);
		
		set_timeout(1000*1000, std::bind(&Consumer::print_stats, this), VNL_TIMER_REPEAT);
		
		vnl::Receiver test(this, address);
		
		log(INFO).out << "Started Consumer" << vnl::endl;
		run();
	}
	
	virtual bool handle(vnl::Packet* pkt) override {
		if(pkt->dst_addr == address) {
			// we got a test_packet_t
			vnl::String& text = ((test_msg_t*)pkt->payload)->text;
			//assert(text == "Hello World");
			pkt->ack();
			counter++;
			return true;
		}
		return false;
	}
	
	void print_stats() {
		log(INFO).out << vnl::dec(counter) << vnl::endl;
		counter = 0;
	}
	
private:
	int counter = 0;
	
};


class Producer : public vnl::Module {
protected:
	virtual void main(vnl::Engine* engine) {
		set_timeout(1000*1000, std::bind(&Producer::print_stats, this), VNL_TIMER_REPEAT);
		test_msg_t test;
		test.text << "Hello World";
		int counter = 0;
		while(poll(0)) {
			for(int k = 0; k < 100; ++k) {
				// publish
				auto* msg = buffer.create<test_msg_t::sample_t>();
				msg->data = test;
				msg->dst_addr = address;
				//send(msg, vnl::Router::instance);
				send_async(msg, vnl::Router::instance);
			}
			flush();
			//std::this_thread::yield();	// for valgrind to switch threads
			counter++;
		}
	}
	
	void print_stats() {
		log(INFO).out << "System: " << vnl::Page::get_num_alloc() << " Pages, " << vnl::Block::get_num_alloc() << " Blocks" << vnl::endl;
	}
	
};


int main() {
	//vnl::Util::stick_to_core(0);
	
	vnl::Layer layer;
	
	vnl::spawn(new Producer());
	for(int i = 0; i < 5; ++i) {
		vnl::spawn(new Consumer());
	}
	for(int i = 0; i < 5; ++i) {
		vnl::fork(new Consumer());
	}
	
	vnl::run(new vnl::Terminal());
	
	std::cout << "Number of pages allocated: " << vnl::Page::get_num_alloc() << std::endl;
	
}










