/*
 * test_module.cpp
 *
 *  Created on: Jul 15, 2016
 *      Author: mad
 */

#include <vnl/ThreadEngine.h>
#include <vnl/Terminal.h>

#include <test/TestNode.h>
#include <test/TroubleMaker.h>


int main() {
	
	vnl::Layer layer("test_module");
	
	usleep(1000*100);
	
	vnl::spawn(new test::TroubleMaker("default", "test/trouble"));
	vnl::spawn(new test::TestNode("default", "test/node"));
	
	vnl::run(new vnl::Terminal());
	
}


