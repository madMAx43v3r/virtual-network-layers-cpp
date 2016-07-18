/*
 * TroubleMaker.h
 *
 *  Created on: Jul 18, 2016
 *      Author: mad
 */

#ifndef TEST_TEST_TROUBLEMAKER_H_
#define TEST_TEST_TROUBLEMAKER_H_

#include <vnl/Module.h>
#include <test/TestType.hxx>


namespace test {

class TroubleMaker : public vnl::Module {
public:
	TroubleMaker(const vnl::String& domain_, const vnl::String& topic_)
		:	Module(domain_, topic_)
	{
		dst = vnl::Address(my_domain, "test/topic");
	}
	
	vnl::Address dst;
	
protected:
	void main(vnl::Engine* engine, vnl::Message* init) {
		init->ack();
		log(INFO).out << "trouble begins ..." << vnl::endl;
		set_timeout(10, std::bind(&TroubleMaker::fire_machine_gun, this), VNL_TIMER_REPEAT);
		run();
	}
	
	void fire_machine_gun() {
		TestType* data = vnl::create<TestType>();
		publish(data, dst);
		std::this_thread::yield();	// for valgrind to switch threads
	}
	
};

}



#endif /* TEST_TEST_TROUBLEMAKER_H_ */
