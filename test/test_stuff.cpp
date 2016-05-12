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

#include "Util.h"
#include "System.h"

#include "phy/Pool.h"
#include "Queue.h"
#include "List.h"
#include "Array.h"
#include "Map.h"
#include "String.h"

#include "../src/phy/Memory.cpp"
#include "../src/util/CRC64.cpp"
#include "../src/String.cpp"

int main() {
	
	int N = 1000;
	
	{
		vnl::phy::Region mem;
		vnl::Queue<int> test(mem);
		for(int iter = 0; iter < N; ++iter) {
			for(int i = 0; i < 100; ++i) {
				test.push(i);
			}
			int c = 0;
			for(int k : test) {
				c++;
			}
			assert(c == 100);
			c = 0;
			for(int k : test) {
				assert(k == c);
				c++;
			}
			for(int i = 0; i < 100; ++i) {
				int k = 0;
				assert(test.pop(k));
				assert(k == i);
			}
			for(int i = 0; i < 100; ++i) {
				test.push(i);
			}
			test.clear();
			for(int k : test) {
				assert(false);
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
				c++;
			}
			assert(c == 100);
			c = 0;
			for(int k : test) {
				assert(k == c);
				c++;
			}
			test.clear();
			for(int k : test) {
				assert(false);
			}
		}
	}
	
	{
		vnl::Map<uint64_t, int> test;
		for(int iter = 0; iter < N; ++iter) {
			std::vector<uint64_t> keys;
			for(int i = 0; i < 100; ++i) {
				uint64_t key = vnl::Util::hash64(rand());
				keys.push_back(key);
				test[key] = i;
			}
			assert(test.size() == 100);
			int i = 0;
			for(auto pair : test.entries()) {
				i++;
			}
			assert(i == 100);
			i = 0;
			for(uint64_t key : keys) {
				int* val = test.find(key);
				assert(val != 0);
				assert(*val == i);
				test.erase(key);
				assert(test.find(key) == 0);
				i++;
			}
			assert(test.size() == 0);
			for(uint64_t key : keys) {
				test[key] = 1337;
			}
			test.clear();
			assert(test.size() == 0);
		}
	}
	
	{
		vnl::String str;
		for(int iter = 0; iter < N; ++iter) {
			std::string std_str;
			for(int i = 0; i < 100; ++i) {
				str << "BLUBB_";
				std_str += "BLUBB_";
			}
			std::ostringstream ss;
			ss << str;
			//std::cout << ss.str() << std::endl;
			//std::cout << str.to_string() << std::endl;
			assert(str.to_string() == std_str);
			assert(ss.str() == std_str);
			str.clear();
		}
	}
	
	{
		vnl::String str("BLUBB");
		//std::cout << str << std::endl;
		assert(str == vnl::String("BLUBB"));
		assert(str.to_string() == std::string("BLUBB"));
		assert(vnl::String(str).to_string() == std::string("BLUBB"));
	}
	
	vnl::phy::Page::cleanup();
	assert(vnl::phy::Page::get_num_alloc() == 0);
	
}


