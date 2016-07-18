/*
 * test_tcp_client.cpp
 *
 *  Created on: Jul 18, 2016
 *      Author: mad
 */

#include <vnl/TcpClient.h>
#include <vnl/ThreadEngine.h>
#include <vnl/Terminal.h>

#include <test/TestNode.h>


int main() {
	
	vnl::Layer layer("test_tcp_client");
	
	vnl::Address addr = vnl::spawn(new vnl::TcpClient("localhost"));
	
	vnl::ThreadEngine engine;
	vnl::TcpClientClient uplink;
	uplink.set_address(addr);
	uplink.connect(&engine);
	
	vnl::Topic topic;
	topic.domain = "test";
	topic.name = "test/topic";
	uplink.subscribe(topic);
	
	vnl::spawn(new test::TestNode("test", "test/node"));
	
	vnl::run(new vnl::Terminal());
	
}


