/*
 * Processor.h
 *
 *  Created on: Jan 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PROCESSOR_H_
#define INCLUDE_PROCESSOR_H_

#include <condition_variable>

#include "Link.h"

namespace vnl { namespace phy {

class Processor : public vnl::phy::Link {
public:
	Processor(Engine* engine) : Link(engine), ulock(mutex) {
		ulock.unlock();
	}
	
protected:
	
	void notify() override {
		cond.notify_all();
	}
	
	void wait(int millis) override {
		cond.wait_for(ulock, std::chrono::milliseconds(millis));
	}
	
private:
	std::condition_variable cond;
	std::unique_lock<std::mutex> ulock;
	
};

}}

#endif /* INCLUDE_PROCESSOR_H_ */
