/*
 * TestNode.h
 *
 *  Created on: Jul 18, 2016
 *      Author: mad
 */

#ifndef TEST_TESTNODE_H_
#define TEST_TESTNODE_H_

#include <test/TestNodeSupport.hxx>


namespace test {

class TestNode : public TestNodeBase {
public:
	TestNode(const vnl::String& domain_, const vnl::String& topic_)
		:	TestNodeBase(domain_, topic_)
	{
	}
	
	int counter = 0;
	int latency = 0;
	
protected:
	void main(vnl::Engine* engine, vnl::Message* init) {
		log(INFO).out << "Hello World: level=" << level << ", instance=" << instance << vnl::endl;
		set_timeout(1000*1000, std::bind(&TestNode::print_stats, this), VNL_TIMER_REPEAT);
		subscribe(vnl::Address(my_domain, "test/topic"));
		init->ack();
		run();
	}
	
	void print_stats() {
		log(INFO).out << "counter = " << vnl::dec(counter) << " latency=" << latency
				<< " (System: " << vnl::Page::get_num_alloc() << " Pages, " << vnl::Block::get_num_alloc() << " Blocks)" << vnl::endl;
		counter = 0;
	}
	
	void handle(const TestType& ev, const vnl::Packet& packet) {
		int64_t now = vnl::currentTimeMicros();
		latency = now - ev.time;
		counter++;
	}
	
	virtual TestValue test_func(const TestValue& val) const {
		TestValue res;
		res.x = 1337;
		res.y = 1338;
		res.z = 1339;
		return res;
	}
	
	virtual test_value_t test_func2(const test_value_t& val) const {
		test_value_t res;
		res.pos[0] = 9;
		res.pos[1] = 99;
		res.pos[2] = 999;
		return res;
	}
	
};

}


#endif /* TEST_TESTNODE_H_ */
