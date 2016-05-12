/*
 * test_atomics.cpp
 *
 *  Created on: May 11, 2016
 *      Author: mad
 */


#include <assert.h>
#include <atomic>
#include <thread>
#include <iostream>
#include <mutex>

struct mydata_t {
	mydata_t* next = 0;
	volatile bool free = true;
};

std::atomic<mydata_t*> begin;
//std::mutex mutex;

class spinlock {
public:
	spinlock() : flag(ATOMIC_FLAG_INIT) {}
	
	void lock() {
		int counter = 0;
		while(flag.test_and_set(std::memory_order_acquire)) {
			counter++;
			if(counter > 1000) {
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
	
};
spinlock mutex;

mydata_t* alloc() {
	mutex.lock();			// locking here WORKS
	mydata_t* data = begin;
	if(data) {
		//mutex.lock();		// locking here doesn't work
		while(!begin.compare_exchange_strong(data, data->next)) {
			if(!data) {
				data = new mydata_t();
				break;
			}
		}
		//mutex.unlock();	// locking here doesn't work
	} else {
		data = new mydata_t();
	}
	mutex.unlock();			// locking here WORKS
	
	/* This is the assert that fails,
	 * indicating that two threads got the same data struct.
	 */
	assert(data->free == true);
	data->free = false;
	return data;
}

void free(mydata_t* data) {
	assert(data->free == false);
	data->free = true;
	data->next = begin;
	while(!begin.compare_exchange_strong(data->next, data)) {}
}

void func() {
	int counter = 0;
	while(true) {
		mydata_t* data = alloc();
		free(data);
		counter++;
		if(counter % (1000*1000) == 0) {
			std::cout << std::this_thread::get_id() << ": " << counter << std::endl;
		}
	}
}

int main() {
	for(int i = 0; i < 10; ++i) {
		std::thread thread(std::bind(&func));
		thread.detach();
	}
	std::this_thread::sleep_for(std::chrono::seconds(100));
}


