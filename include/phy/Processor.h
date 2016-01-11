/*
 * Processor.h
 *
 *  Created on: Jan 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PROCESSOR_H_
#define INCLUDE_PROCESSOR_H_

#include <mutex>
#include <condition_variable>

namespace vnl { namespace phy {

template<typename T>
class Processor : public T {
public:
	Processor(int N) : T(N), ulock(mutex) {
		ulock.unlock();
	}
	
	virtual ~Processor() {}
	
protected:
	
	virtual void notify() override {
		cond.notify_all();
	}
	
	virtual void wait(int millis) override {
		cond.wait_for(ulock, std::chrono::milliseconds(millis));
	}
	
private:
	std::condition_variable cond;
	std::unique_lock<std::mutex> ulock;
	
};

}}

#endif /* INCLUDE_PROCESSOR_H_ */
