/*
 * Page.h
 *
 *  Created on: Mar 5, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_MEMORY_H_
#define INCLUDE_PHY_MEMORY_H_

#include <stdlib.h>
#include <assert.h>
#include <atomic>
#include <iostream>

#include "vnl/build/config.h"
#include "vnl/util/spinlock.h"


namespace vnl {

class Page {
public:
	static const int size = VNL_PAGE_SIZE;
	
	Page(const Page&) = delete;
	Page& operator=(const Page&) = delete;
	
	static Page* alloc();
	
	void free();
	
	void free_all();
	
	template<typename T>
	T& get(int index) {
		return *(((T*)mem) + index);
	}
	
	static size_t get_num_alloc() {
		return num_alloc;
	}
	
	static void cleanup();
	
	char* mem;
	
	Page* next;
	
private:
	Page() : next(0) {
		mem = (char*)malloc(size);
		assert(mem != OUT_OF_MEMORY);
		num_alloc++;
	}
	
	~Page() {
		::free(mem);
		num_alloc--;
	}
	
private:
	static const int OUT_OF_MEMORY = 0;
	static util::spinlock mutex;
	static Page* begin;
	static size_t num_alloc;
	
};


class Region {
public:
	Region() {}
	
	~Region() {
		free_all();
	}
	
	Region(const Region&) = delete;
	Region& operator=(const Region&) = delete;
	
	template<typename T>
	T* create() {
		return new(alloc<T>()) T();
	}
	
	template<typename T>
	void* alloc() {
#ifndef VNL_MEMORY_DEBUG
		return alloc(sizeof(T));
#else
		return malloc(sizeof(T));
#endif
	}
	
	void* alloc(int size);
	
	Page* front() {
		return p_front;
	}
	
	void clear();
	
	void free_all() {
		if(p_front) {
			p_front->free_all();
			p_front = 0;
			p_back = 0;
			pos = 0;
		}
	}
	
private:
	Page* p_front = 0;
	Page* p_back = 0;
	int pos = 0;
	
};



}

#endif /* INCLUDE_PHY_MEMORY_H_ */
