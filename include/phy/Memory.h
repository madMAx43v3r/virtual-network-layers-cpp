/*
 * Page.h
 *
 *  Created on: Mar 5, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_MEMORY_H_
#define INCLUDE_PHY_MEMORY_H_

#include "util/spinlock.h"

#ifndef VNL_PHY_PAGESIZE
#define VNL_PHY_PAGESIZE 4096
#endif


namespace vnl { namespace phy {

class Page {
public:
	static const int size = VNL_PHY_PAGESIZE;
	
	static Page* alloc() {
		sync.lock();
		if(begin) {
			Page* page = begin;
			begin = begin->next;
			sync.unlock();
			page->next = 0;
			return page;
		} else {
			sync.unlock();
			return new Page(this);
		}
	}
	
	Page(const Page&) = delete;
	Page& operator=(const Page&) = delete;
	
	void free() {
		sync.lock();
		_free();
		sync.unlock();
	}
	
	void free_all() {
		sync.lock();
		_free_all();
		sync.unlock();
	}
	
	char* mem;
	
	Page* next;
	
private:
	Page() : next(0) {
		mem = (char*)malloc(size);
	}
	
	~Page() {
		::free(mem);
	}
	
	void _free() {
		next = begin;
		begin = this;
	}
	
	void _free_all() {
		Page* page = this;
		while(page) {
			Page* tmp = page;
			page = page->next;
			tmp->_free();
		}
	}
	
private:
	static vnl::util::spinlock sync;
	static Page* begin;
	
};


class Region {
public:
	Region() {
		p_front = Page::alloc();
		p_back = p_front;
		left = Page::size;
	}
	
	Region(size_t bytes) : Region() {
		size_t n = bytes/Page::size;
		Page* page = p_back;
		for(size_t i = 0; i < n; ++i) {
			page->next = Page::alloc();
			page = page->next;
		}
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
		return alloc(sizeof(T));
	}
	
	void* alloc(int size) {
		assert(size <= Page::size);
		if(left < size) {
			if(!p_back->next) {
				p_back->next = Page::alloc();
			}
			p_back = p_back->next;
			left = Page::size;
		}
		void* ptr = p_back->mem + (Page::size - left);
		left -= size;
		return ptr;
	}
	
private:
	Page* p_front;
	Page* p_back;
	int left;
	
};



}}

#endif /* INCLUDE_PHY_MEMORY_H_ */
