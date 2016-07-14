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

#include "vnl/Util.h"

#include "vnl/Pool.h"
#include "vnl/Queue.h"
#include "vnl/List.h"
#include "vnl/Array.h"
#include "vnl/Map.h"
#include "vnl/String.h"
#include "vnl/Algorithm.h"

#include "../src/CRC64.cpp"

int main() {
	
	int N = 100;
	int M = 10000;
	
	{
		vnl::Queue<int> test;
		for(int iter = 0; iter < N; ++iter) {
			for(int i = 0; i < M; ++i) {
				test.push(i);
			}
			int c = 0;
			for(int k : test) {
				c++;
			}
			assert(c == M);
			c = 0;
			for(int k : test) {
				assert(k == c);
				c++;
			}
			for(int i = 0; i < M; ++i) {
				int k = 0;
				assert(test.pop(k));
				assert(k == i);
			}
			for(int i = 0; i < M; ++i) {
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
			for(int i = 0; i < M; ++i) {
				test.push_back(i);
			}
			assert(test.size() == M);
			int c = 0;
			for(int k : test) {
				assert(k == c);
				c++;
			}
			assert(c == M);
			for(int i = 0; i < M; ++i) {
				assert(test[i] == i);
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
			for(int i = 0; i < M; ++i) {
				uint64_t key = vnl::hash64(i);
				keys.push_back(key);
				assert(test.find(key) == 0);
				test[key] = i;
				assert(test.size() == i+1);
			}
			int i = 0;
			for(auto pair : test.entries()) {
				i++;
			}
			assert(i == M);
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
			for(int i = 0; i < M; ++i) {
				str << "BLUBB_";
				std_str += "BLUBB_";
			}
			std::ostringstream ss;
			ss << str;
			//std::cout << ss.str() << std::endl;
			//std::cout << str.to_string() << std::endl;
			assert(str.to_string() == std_str);
			assert(ss.str() == std_str);
			assert(str == str);
			assert(vnl::String(str) == str);
			str.clear();
		}
	}
	
	{
		vnl::String str;
		str << "BLUBB";
		std::cout << str << std::endl;
		assert(str == str);
		assert(str == (vnl::String() << "BLUBB"));
		assert(str.to_string() == std::string("BLUBB"));
	}
	
	{
		vnl::Array<int> test;
		test.push_back(4);
		test.push_back(3);
		test.push_back(2);
		test.push_back(1);
		test.push_back(2);
		vnl::sort(test.begin(), test.end());
		assert(test[0] == 1);
		assert(test[1] == 2);
		assert(test[2] == 2);
		assert(test[3] == 3);
		assert(test[4] == 4);
	}
	
	vnl::Page::cleanup();
	vnl::Block::cleanup();
	assert(vnl::Page::get_num_alloc() == 0);
	assert(vnl::Block::get_num_alloc() == 0);
	
}


