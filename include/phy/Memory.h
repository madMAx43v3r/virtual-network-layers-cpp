/*
 * Page.h
 *
 *  Created on: Mar 5, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_MEMORY_H_
#define INCLUDE_PHY_MEMORY_H_

#include <atomic>

#ifndef VNL_PHY_PAGE_SIZE
#define VNL_PHY_PAGE_SIZE 4096
#endif

#ifndef VNL_PHY_MEMORY_DEBUG
#define VNL_PHY_MEMORY_DEBUG false
#endif


namespace vnl { namespace phy {

class Page {
public:
	static const int size = VNL_PHY_PAGE_SIZE;
	
	static size_t num_alloc;
	
	static Page* alloc() {
		num_alloc++;
		Page* page = begin.load(std::memory_order_acquire);
		if(page) {
			while(!begin.compare_exchange_strong(page, page->next, std::memory_order_acq_rel)) {
				if(!page) {
					return new Page();
				}
			}
			page->next = 0;
			return page;
		} else {
			return new Page();
		}
	}
	
	Page(const Page&) = delete;
	Page& operator=(const Page&) = delete;
	
	void free() {
		num_alloc--;
		next = begin.load(std::memory_order_acquire);
		while(!begin.compare_exchange_strong(next, this, std::memory_order_acq_rel)) {}
	}
	
	void free_all() {
		Page* page = this;
		while(page) {
			Page* tmp = page;
			page = page->next;
			tmp->free();
		}
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
	
private:
	static std::atomic<Page*> begin;
	
};


template<typename T>
class TPage : public Page {
public:
	static const int M = Page::size / sizeof(T);
	
	static TPage* alloc() {
		return (TPage*)Page::alloc();
	}
	
	T& operator[](int index) {
		return *(((T*)mem) + index);
	}
	
};


class Region {
public:
	static const bool debug = VNL_PHY_MEMORY_DEBUG;
	
	Region() {
		p_front = Page::alloc();
		p_back = p_front;
		pos = 0;
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
		if(debug) {
			return new T();
		} else {
			return new(alloc<T>()) T();
		}
	}
	
	template<typename T>
	void* alloc() {
		if(debug) {
			return malloc(sizeof(T));
		} else {
			return alloc(sizeof(T));
		}
	}
	
	void* alloc(int size) {
		assert(size <= Page::size);
		if(Page::size - pos < size) {
			if(!p_back->next) {
				p_back->next = Page::alloc();
			}
			p_back = p_back->next;
			pos = 0;
		}
		void* ptr = p_back->mem + pos;
		pos += size;
		return ptr;
	}
	
	void clear() {
		p_back = p_front;
		pos = 0;
	}
	
private:
	Page* p_front;
	Page* p_back;
	int pos;
	
};



}}

#endif /* INCLUDE_PHY_MEMORY_H_ */
