/*
 * test_phy_router.cpp
 *
 *  Created on: May 8, 2016
 *      Author: mad
 */

#include "phy/Layer.h"
#include "phy/Router.h"
#include "phy/ThreadEngine.h"
#include "phy/Stream.h"

#include "../src/util/CRC64.cpp"
#include "../src/phy/Engine.cpp"
#include "../src/phy/Memory.cpp"
#include "../src/phy/Message.cpp"
#include "../src/phy/Random.cpp"
#include "../src/phy/Router.cpp"
#include "../src/phy/Object.cpp"
#include "../src/phy/Registry.cpp"
#include "../src/String.cpp"

/*
 * Showcasing the basic vnl::phy functionality.
 */

struct test_msg_t {
	vnl::String text;
	VNL_SAMPLE(test_msg_t);
};

vnl::Address address("domain", "topic");

class Consumer : public vnl::phy::Object {
protected:
	virtual void main(vnl::phy::Engine* engine) override {
		//vnl::Util::stick_to_core(mac % 3 + 1);
		
		timeout(1000*1000, std::bind(&Consumer::print_stats, this, std::placeholders::_1), vnl::phy::Timer::REPEAT);
		
		vnl::phy::Receiver test(this, address);
		
		run();
	}
	
	virtual bool handle(vnl::phy::Packet* pkt) override {
		if(pkt->dst_addr == address) {
			// we got a test_packet_t
			vnl::String& text = ((test_msg_t*)pkt->payload)->text;
			assert(text == "Hello World");
			pkt->ack();
			counter++;
			return true;
		}
		return false;
	}
	
	void print_stats(vnl::phy::Timer* timer) {
		std::cout << mac << " " << counter << std::endl;
		counter = 0;
	}
	
private:
	int counter = 0;
	
};


int main() {
	//vnl::Util::stick_to_core(0);
	
	vnl::phy::Layer layer;
	vnl::phy::ThreadEngine engine;
	
	vnl::phy::Region mem;
	vnl::phy::Stream pub(&engine, mem);
	
	Consumer* consumer;
	for(int i = 0; i < 2; ++i) {
		consumer = new Consumer();
		engine.fork(consumer);
	}
	
	vnl::phy::MessageBuffer buffer(mem);
	
	test_msg_t test;
	test.text = "Hello World";
	
	int counter = 0;
	while(counter < 1000*1000) {
		
		for(int k = 0; k < 100; ++k) {
			// publish
			test_msg_t::sample_t* msg = buffer.create<test_msg_t::sample_t>(test, address);
			//pub.send(msg, vnl::phy::Router::instance);
			pub.send_async(msg, vnl::phy::Router::instance);
		}
		
		pub.flush();
		//std::this_thread::yield();	// for valgrind to switch threads
		counter++;
		
	}
	
	std::cout << "Number of pages allocated: " << vnl::phy::Page::get_num_alloc() << std::endl;
	
	layer.shutdown();
	
}










