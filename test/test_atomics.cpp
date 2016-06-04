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
	std::atomic<bool> free;
	mydata_t() { free = true; }
};

std::atomic<mydata_t*> begin;
std::mutex mutex;

mydata_t* alloc() {
	//mutex.lock();			// locking here WORKS
	mydata_t* data = begin;
	if(data) {
		mutex.lock();		// locking here doesn't work
		while(!begin.compare_exchange_strong(data, data->next)) {
			if(!data) {
				data = new mydata_t();
				break;
			}
		}
		mutex.unlock();		// locking here doesn't work
	} else {
		data = new mydata_t();
	}
	//mutex.unlock();		// locking here WORKS
	
	mutex.lock();
	if(data->free != true) {
		std::cout << "data = " << data << std::endl;
		mutex.unlock();
		return 0;
	}
	data->free = false;
	data->next = 0;
	mutex.unlock();
	return data;
}

void free(mydata_t* data) {
	//mutex.lock();
	data->free = true;
	data->next = begin;
	while(!begin.compare_exchange_strong(data->next, data)) {}
	//mutex.unlock();
}

void func() {
	int counter = 0;
	while(true) {
		mydata_t* data = alloc();
		if(!data) {
			mutex.lock();
			std::cout << "counter = " << counter << std::endl;
			mutex.unlock();
			std::this_thread::sleep_for(std::chrono::seconds(1));
			assert(data != 0);
		}
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


