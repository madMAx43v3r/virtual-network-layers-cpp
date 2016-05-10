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
 * Showcasing the basic vnl::phy functionality regarding the vnl::phy::Router.
 * This example will send 1M messages through the router.
 * Note that this entire program only makes ~30 calls to malloc().
 */

typedef vnl::phy::PacketType<std::pair<int, vnl::String> > test_packet_t;

vnl::Address address("domain", "topic");

class Consumer : public vnl::phy::Object {
public:
	Consumer(vnl::phy::Router* router) : router(router) {}
	
protected:
	virtual bool startup() override {
		vnl::Util::stick_to_core(2);
		// subscribe
		std::cout << "Consumer " << mac << ": subscribe " << address << std::endl;
		send(vnl::phy::Router::connect_t(address), router);
		return true;
	}
	
	virtual void shutdown() override {
		// unsubscribe
		std::cout << "Consumer " << mac << ": unsubscribe " << address << std::endl;
		send(vnl::phy::Router::close_t(address), router);
	}
	
	virtual bool handle(vnl::phy::Message* msg) override {
		// see what type of message we got
		if(msg->mid == vnl::phy::Router::packet_t::id) {
			// we got a packet from a router
			vnl::phy::Packet* packet = ((vnl::phy::Router::packet_t*)msg)->data.packet;
			if(packet->dst == address) {
				// we got a test_packet_t
				test_packet_t* test = dynamic_cast<test_packet_t*>(packet);
				if(test) {
					std::pair<int, vnl::String>& payload = test->payload;
					if(payload.first % (1000*1000) == 0) {
						std::cout << vnl::System::currentTimeMillis() << " " << payload.first << " " << payload.second << std::endl;
					}
					assert(payload.second == "Hello World");
					msg->ack();
					return true;
				} else {
					std::cout << "ERROR: dynamic_cast<test_packet_t*> failed" << std::endl;
				}
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
	
	for(int i = 0; i < 3; ++i) {
		engine.fork(new Consumer(&router));
	}
	
	int counter = 0;
	for(int i = 0; i < 1000*1000*1000; ++i) {
		
		// publish
		test_packet_t message(std::make_pair(counter++, "Hello World"));
		pub.send(vnl::phy::Router::packet_t(&message, address), &router);
		
	}
	
	std::cout << "Number of pages allocated: " << vnl::phy::Page::get_num_alloc() << std::endl;
	
	
	
}










