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

typedef vnl::phy::PacketType<vnl::String> test_packet_t;

int main() {
	
	vnl::phy::Layer layer;
	vnl::phy::Router router;
	vnl::phy::ThreadEngine engine;
	
	vnl::phy::Region mem;
	vnl::phy::Stream pub(&engine, mem);
	vnl::phy::Stream sub(&engine, mem);
	
	vnl::Address address("domain", "topic");
	
	// subscribe
	sub.send(vnl::phy::Router::connect_t(address), &router);
	
	for(int i = 0; i < 1000*1000; ++i) {
		
		// publish
		test_packet_t message("Hello World");
		pub.send_async(vnl::phy::Router::packet_t(&message, address), &router);
		
		// see if we got anything
		while(true) {
			vnl::phy::Message* msg = sub.poll(0);
			if(!msg) {
				break;
			}
			if(msg->mid == vnl::phy::Router::packet_t::id) {
				// we got a packet
				vnl::phy::Packet* packet = ((vnl::phy::Router::packet_t*)msg)->data.payload;
				if(packet->dst == address) {
					// we got a test_packet_t
					vnl::String payload = ((test_packet_t*)packet)->data;
					if(i == 0) {
						std::cout << payload << std::endl;
					}
					assert(payload == "Hello World");
				}
			}
			msg->ack();
		}
		
	}
	
	std::cout << "Number of pages allocated: " << vnl::phy::Page::get_num_alloc() << std::endl;
	
	// unsubscribe
	sub.send(vnl::phy::Router::close_t(address), &router);
	
}

