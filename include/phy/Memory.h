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

#ifndef VNL_PAGE_SIZE
#define VNL_PAGE_SIZE 4096
#endif

//#define VNL_MEMORY_DEBUG


namespace vnl { namespace phy {

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
	static std::atomic<Page*> begin;
	static size_t num_alloc;
	
};


class Region {
public:
	Region() {
		p_front = Page::alloc();
		p_back = p_front;
		pos = 0;
	}
	
	~Region() {
		p_front->free_all();
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
		return malloc(sizeof(T);
#endif
	}
	
	void* alloc(int size);
	
	void clear();
	
private:
	Page* p_front;
	Page* p_back;
	int pos;
	
};



}}

#endif /* INCLUDE_PHY_MEMORY_H_ */
