/*
 * spinlock.h
 *
 *  Created on: Dec 16, 2015
 *      Author: mad
 */

#ifndef INCLUDE_SPINLOCK_H_
#define INCLUDE_SPINLOCK_H_

#include <atomic>

namespace vnl { namespace util {

class spinlock {
public:
	spinlock() : flag(ATOMIC_FLAG_INIT) {}
	
	void lock() {
		while(flag.test_and_set(std::memory_order_acquire)) {
			std::this_thread::yield();
		}
	}
	
	void unlock() {
		flag.clear(std::memory_order_release);
	}
	
private:
	std::atomic_flag flag;
	
};

}}

#endif /* INCLUDE_SPINLOCK_H_ */
