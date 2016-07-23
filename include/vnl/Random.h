/*
 * Random.h
 *
 *  Created on: May 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_RANDOM_H_
#define INCLUDE_PHY_RANDOM_H_

#include <random>
#include <mutex>
#include <thread>

#include <vnl/Util.h>


namespace vnl {

class Random64 {
public:
	static Random64* instance;
	
	Random64() {
		generator.seed(hash64((uint64_t)std::hash<std::thread::id>{}(std::this_thread::get_id()), (uint64_t)nanoTime()));
	}
	
	uint64_t rand() {
		sync.lock();
		uint64_t val = generator();
		sync.unlock();
		return val;
	}
	
	static uint64_t global_rand() {
		if(!instance) {
			instance = new Random64();
		}
		return instance->rand();
	}
	
private:
	std::mt19937_64 generator;
	
	std::mutex sync;
	
};


}

#endif /* INCLUDE_PHY_RANDOM_H_ */
