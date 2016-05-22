/*
 * Random.h
 *
 *  Created on: May 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_RANDOM_H_
#define INCLUDE_PHY_RANDOM_H_

#include <random>
#include <thread>

#include "util/spinlock.h"
#include "System.h"
#include "Util.h"


namespace vnl { namespace phy {

class Layer;


class Random64 {
public:
	Random64() {
		generator.seed(vnl::hash64((uint64_t)counter++, (uint64_t)std::hash<std::thread::id>{}(std::this_thread::get_id()), (uint64_t)vnl::nanoTime()));
	}
	
	uint64_t rand() {
		return generator();
	}
	
	static uint64_t global_rand() {
		sync.lock();
		uint64_t val = instance->rand();
		sync.unlock();
		return val;
	}
	
private:
	std::mt19937_64 generator;
	
	static vnl::util::spinlock sync;
	static Random64* instance;
	static std::atomic<int> counter;
	
	friend class Layer;
	
};


}}

#endif /* INCLUDE_PHY_RANDOM_H_ */
