/*
 * test_yield.cpp
 *
 *  Created on: Dec 16, 2015
 *      Author: mad
 */

#include <assert.h>
#include <thread>
#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <cstdlib>
#include <atomic>

#include "phy/Util.h"
#include "phy/System.h"

#include "Queue.h"
#include "List.h"
#include "Array.h"
#include "Map.h"
#include "String.h"

#include "../src/phy/Memory.cpp"
#include "../src/util/CRC64.cpp"

int main() {
	
	int N = 1000;
	
	{
		vnl::phy::Region mem;
		vnl::Queue<int> test(&mem);
		for(int iter = 0; iter < N; ++iter) {
			for(int i = 0; i < 100; ++i) {
				test.push(i);
			}
			int c = 0;
			for(int k : test) {
				assert(k == c);
				c++;
			}
			for(int i = 0; i < 100; ++i) {
				int k = 0;
				assert(test.pop(k));
				assert(k == i);
			}
		}
	}
	
	{
		vnl::Array<int> test;
		for(int iter = 0; iter < N; ++iter) {
			for(int i = 0; i < 100; ++i) {
				test.push_back(i);
			}
			int c = 0;
			for(int k : test) {
				assert(k == c);
				c++;
			}
			test.clear();
		}
	}
	
	{
		vnl::Map<uint64_t, int> test;
		for(int iter = 0; iter < N; ++iter) {
			std::vector<uint64_t> keys;
			for(int i = 0; i < 100; ++i) {
				uint64_t key = vnl::phy::Util::hash64(rand());
				keys.push_back(key);
				test[key] = i;
			}
			assert(test.size() == 100);
			int i = 0;
			for(uint64_t key : keys) {
				int* val = test.find(key);
				assert(val != 0);
				assert(*val == i);
				test.erase(key);
				assert(test.find(key) == 0);
				i++;
			}
			assert(test.size() == 0);
			i = 0;
			for(uint64_t key : keys) {
				test[key] = i;
				i++;
			}
			test.clear();
			assert(test.size() == 0);
		}
	}
	
	{
		vnl::phy::Region mem;
		vnl::String str(&mem);
		for(int i = 0; i < 10; ++i) {
			str << "BLUBB_" << 1;
			str << std::string("BLUBB_") << 1.1;
		}
		std::cout << str << std::endl;
		std::cout << str.to_str() << std::endl;
	}
	
	std::cout << "Pages still allocated: " << vnl::phy::Page::num_alloc << std::endl;
	assert(vnl::phy::Page::num_alloc == 0);
	
}


