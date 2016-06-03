/*
 * spinlock.h
 *
 *  Created on: Dec 16, 2015
 *      Author: mad
 */

#ifndef INCLUDE_SPINLOCK_H_
#define INCLUDE_SPINLOCK_H_

#include <atomic>
#include <thread>


namespace vnl { namespace util {

class spinlock {
public:
	spinlock(int max_spin = 1000) : flag(ATOMIC_FLAG_INIT), max_spin(max_spin) {}
	
	void lock() {
		int counter = 0;
		while(flag.test_and_set(std::memory_order_acquire)) {
			counter++;
			if(counter > max_spin) {
				std::this_thread::yield();
				counter = 0;
			}
		}
	}
	
	void unlock() {
		flag.clear(std::memory_order_release);
	}
	
private:
	std::atomic_flag flag;
	int max_spin;
	
};

}}

#endif /* INCLUDE_SPINLOCK_H_ */