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

typedef vnl::phy::SampleType<vnl::String> test_packet_t;

vnl::Address address("domain", "topic");

class Consumer : public vnl::phy::Object {
protected:
	virtual void main(vnl::phy::Engine* engine) override {
		//vnl::Util::stick_to_core(mac % 3 + 1);
		
		timeout(1000*1000, std::bind(&Consumer::print_stats, this, std::placeholders::_1), vnl::phy::Timer::REPEAT);
		
		// subscribe
		std::cout << "Consumer " << mac << ": subscribe " << address << std::endl;
		connect(address);
		
		run();
		
		// unsubscribe
		std::cout << "Consumer " << mac << ": unsubscribe " << address << std::endl;
		close(address);
	}
	
	virtual bool handle(vnl::phy::Message* msg) override {
		// see what type of message we got
		if(msg->msg_id == vnl::phy::Packet::MID) {
			// we got a packet from a router
			vnl::phy::Packet* packet = (vnl::phy::Packet*)msg;
			if(packet->dst_addr == address && packet->pkt_id == vnl::phy::SAMPLE) {
				// we got a test_packet_t
				const vnl::String* payload = (test_packet_t::data_t*)packet->payload;
				assert(*payload == "Hello World");
				msg->ack();
				counter++;
				return true;
			}
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
	
	int counter = 0;
	while(counter < 1000) {
		
		for(int k = 0; k < 100; ++k) {
			// publish
			test_packet_t* msg = buffer.create<test_packet_t>("Hello World", address);
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










