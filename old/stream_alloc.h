/*
 * stream_alloc.h
 *
 *  Created on: Dec 22, 2015
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_STREAM_ALLOC_H_
#define INCLUDE_UTIL_STREAM_ALLOC_H_

#include <exception>
#include <new>
#include <stdlib.h>
#include <atomic>

//#define USE_VNL_ALLOC

namespace vnl { namespace util { namespace alloc {

static const uintptr_t PAGE_SIZE = 4096;
static const uintptr_t ALIGNMENT = 4;
static const uintptr_t DATA_SIZE = PAGE_SIZE - ALIGNMENT;

std::atomic<int> stat_num_pages;

struct page_t {
	std::atomic<uint16_t> used;
	char data[DATA_SIZE];
};

page_t* alloc_page() {
	page_t* page = (page_t*)aligned_alloc(PAGE_SIZE, PAGE_SIZE);
	page->used = 0;
	stat_num_pages++;
	return page;
}

thread_local page_t* _page = alloc_page();



}}}

#ifdef USE_VNL_ALLOC

void* operator new(size_t size) {
	using namespace vnl::util::alloc;
	if(size <= PAGE_SIZE/64) {
		size += 2;
		size += ALIGNMENT - (size % ALIGNMENT);
		if(DATA_SIZE - _page->used < size) {
			_page = alloc_page();
		}
		uint16_t index = _page->used.fetch_add(size);
		uint16_t* ptr = (uint16_t*)&_page->data[index];
		*ptr = size;
		return ptr+1;
	} else {
		size += 2;
		size += sizeof(size_t) - (size % sizeof(size_t));
		void* p = aligned_alloc(sizeof(size_t), size);
		if(p == 0) throw std::bad_alloc();
		return (void*)(uintptr_t(p) + 2);
	}
}

void operator delete(void* p) {
	using namespace vnl::util::alloc;
	uintptr_t ptr = (uintptr_t)p;
	if(ptr % ALIGNMENT == 0) {
		uint16_t size = *(((uint16_t*)p)-1);
		page_t* page = (page_t*)(ptr & ~(PAGE_SIZE-1));
		uint16_t prev = page->used.fetch_sub(size);
		if(prev == size) {
			free((void*)page);
		}
	} else {
		free((void*)(ptr - 2));
	}
}

#endif







#endif /* INCLUDE_UTIL_STREAM_ALLOC_H_ */
