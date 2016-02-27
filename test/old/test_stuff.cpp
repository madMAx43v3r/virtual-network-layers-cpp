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
#include "Util.h"
#include "System.h"

#include "util/simple_queue.h"
#include "util/simple_hashmap.h"


int main() {
	
	{
		vnl::util::simple_queue<int> test;
		for(int i = 0; i < 100; ++i) {
			test.push(i);
		}
		std::cout << "size = " << test.size() << std::endl;
		int sum = 0;
		for(int i : test) {
			sum += i;
		}
		std::cout << "sum = " << sum << std::endl;
		for(int i = 0; i < 200; ++i) {
			int k = i;
			test.pop(k);
			assert(k == i);
		}
		std::cout << "size = " << test.size() << std::endl;
		for(int i = 0; i < 100; ++i) {
			test.push(i);
		}
		for(auto iter = test.begin(); iter != test.end(); ++iter) {
			if(*iter == 50) {
				test.erase(iter);
			}
		}
		std::cout << "size = " << test.size() << std::endl;
	}
	
	{
		vnl::util::simple_hashmap<uint64_t, int> test;
		std::vector<uint64_t> keys;
		for(int i = 0; i < 100; ++i) {
			uint64_t key = vnl::Util::hash64(rand());
			keys.push_back(key);
			test.put(key, i);
		}
		std::cout << "size = " << test.size() << std::endl;
		int i = 0;
		for(uint64_t key : keys) {
			int* val = test.get(key);
			assert(val != 0);
			assert(*val == i);
			test.erase(key);
			assert(test.get(key) == 0);
			i++;
		}
		std::cout << "size = " << test.size() << std::endl;
		for(auto iter = test.begin(); iter != test.end(); ++iter) {
			printf("blubb\n");
		}
		i = 0;
		for(uint64_t key : keys) {
			test.put(key, i);
			i++;
		}
		for(auto iter = test.begin(); iter != test.end(); ++iter) {
			if(iter->val == 50) {
				test.erase(iter);
			}
		}
		std::cout << "size = " << test.size() << std::endl;
	}
	
	
	
}


