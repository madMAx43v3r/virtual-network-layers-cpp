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

class Layer;

class Random64 {
public:
	Random64() {
		generator.seed(hash64((uint64_t)std::hash<std::thread::id>{}(std::this_thread::get_id()), (uint64_t)nanoTime()));
	}
	
	uint64_t rand() {
		return generator();
	}
	
	static uint64_t global_rand() {
		assert(instance);
		static std::mutex sync;
		sync.lock();
		uint64_t value = instance->rand();
		sync.unlock();
		return value;
	}
	
private:
	std::mt19937_64 generator;
	
	static Random64* instance;
	
	friend class Layer;
	
};


}

#endif /* INCLUDE_PHY_RANDOM_H_ */
