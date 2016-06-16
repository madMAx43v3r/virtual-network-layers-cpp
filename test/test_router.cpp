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
#include "vnl/Node.h"

#include "../src/CRC64.cpp"
#include "../src/Engine.cpp"
#include "../src/Message.cpp"
#include "../src/Random.cpp"
#include "../src/Module.cpp"
#include "../src/Registry.cpp"
#include "../src/Router.cpp"
#include "../src/String.cpp"
#include "../src/FiberEngine.cpp"


struct test_msg_t {
	vnl::String text;
	VNL_SAMPLE(test_msg_t);
};

vnl::Address address(vnl::hash64("domain"), vnl::hash64("topic"));

class Consumer : public vnl::Node {
protected:
	virtual void main(vnl::Engine* engine) override {
		//vnl::Util::stick_to_core(mac % 3 + 1);
		
		set_timeout(1000*1000, std::bind(&Consumer::print_stats, this), vnl::Timer::REPEAT);
		
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


#ifdef VNL_HAVE_FIBER_ENGINE

class Core : public vnl::Node {
protected:
	virtual void main(vnl::Engine* engine) override {
		vnl::FiberEngine local;
		for(int i = 0; i < 5; ++i) {
			local.fork(new Consumer());
		}
		local.run();
	}
};

#endif


int main() {
	//vnl::Util::stick_to_core(0);
	
	vnl::Layer layer;
	vnl::ThreadEngine engine;
	
	vnl::Stream pub(&engine);
	
#ifdef VNL_HAVE_FIBER_ENGINE
	engine.fork(new Core());
#endif
	for(int i = 0; i < 5; ++i) {
		engine.fork(new Consumer());
	}
	
	vnl::MessageBuffer buffer;
	
	test_msg_t test;
	test.text << "Hello World";
	
	int counter = 0;
	while(counter < 1000*1000) {
		
		for(int k = 0; k < 100; ++k) {
			// publish
			auto* msg = buffer.create<test_msg_t::sample_t>();
			msg->data = test;
			msg->dst_addr = address;
			//pub.send(msg, vnl::phy::Router::instance);
			pub.send_async(msg, vnl::Router::instance);
		}
		
		pub.flush();
		std::this_thread::yield();	// for valgrind to switch threads
		counter++;
		
	}
	
	std::cout << "Number of pages allocated: " << vnl::Page::get_num_alloc() << std::endl;
	
}










