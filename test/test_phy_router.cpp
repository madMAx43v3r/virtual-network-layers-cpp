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

typedef vnl::phy::PacketType<std::pair<int, vnl::String>, 0x1b343d38> test_packet_t;

vnl::Address address("domain", "topic");

class Consumer : public vnl::phy::Object {
public:
	Consumer(vnl::phy::Router* router) : router(router) {}
	
protected:
	virtual void main() override {
		vnl::Util::stick_to_core(1);
		
		// subscribe
		std::cout << "Consumer " << mac << ": subscribe " << address << std::endl;
		vnl::phy::Router::connect_t connect(address);
		send(&connect, router);
		
		run();
		
		// unsubscribe
		std::cout << "Consumer " << mac << ": unsubscribe " << address << std::endl;
		vnl::phy::Router::close_t close(address);
		send(&close, router);
	}
	
	virtual bool handle(vnl::phy::Message* msg) override {
		// see what type of message we got
		if(msg->mid == vnl::phy::Packet::MID) {
			// we got a packet from a router
			vnl::phy::Packet* packet = (vnl::phy::Packet*)msg;
			if(packet->pid == test_packet_t::PID) {
				// we got a test_packet_t
				const std::pair<int, vnl::String>* payload = (test_packet_t::data_t*)packet->payload;
				if(payload->first % (1000*1000) == 0) {
					std::cout << vnl::System::currentTimeMillis() << " " << payload->first << " " << payload->second << std::endl;
				}
				assert(payload->second == "Hello World");
				msg->ack();
				return true;
			}
		}
		return false;
	}
	
private:
	vnl::phy::Router* router;
	
};


int main() {
	vnl::Util::stick_to_core(0);
	
	vnl::phy::Layer layer;
	vnl::phy::Router router;
	vnl::phy::ThreadEngine engine;
	
	vnl::phy::Region mem;
	vnl::phy::Stream pub(&engine, mem);
	
	Consumer* consumer;
	for(int i = 0; i < 1; ++i) {
		consumer = new Consumer(&router);
		engine.fork(consumer);
	}
	
	vnl::phy::MessageBuffer buffer(mem);
	
	int counter = 0;
	while(counter < 1000*1000) {
		
		for(int k = 0; k < 100; ++k) {
			// publish
			test_packet_t* msg = buffer.create<test_packet_t>(std::make_pair(counter++, "Hello World"), address);
			//pub.send(msg, &router);
			pub.send_async(msg, &router);
		}
		
		pub.flush();
		//std::this_thread::yield();	// for valgrind to switch threads
		
	}
	
	std::cout << "Number of pages allocated: " << vnl::phy::Page::get_num_alloc() << std::endl;
	
	
	layer.shutdown();
}










