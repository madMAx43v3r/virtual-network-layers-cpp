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

template<int size_>
class Memory {
public:
	static const int size = size_;
	
	Memory(const Memory&) = delete;
	Memory& operator=(const Memory&) = delete;
	
#ifdef VNL_MEMORY_DEBUG
	static Memory* alloc() {
		return new Memory();
	}
	
	void free() {
		delete this;
	}
#else
	static Memory* alloc() {
		sync.lock();
		Memory* page = begin;
		if(page) {
			begin = page->next;
		}
		sync.unlock();
		if(!page) {
			page = new Memory();
			num_alloc++;
		}
		assert(page != OUT_OF_MEMORY);
		page->next = 0;
		return page;
	}
	
	void free() {
		sync.lock();
		next = begin;
		begin = this;
		sync.unlock();
	}
#endif
	
	void free_all() {
		Memory* page = this;
		while(page) {
			Memory* tmp = page;
			page = page->next;
			tmp->free();
		}
	}
	
	template<typename T>
	static T* alloc_ex() {
		assert(sizeof(T) == sizeof(Memory));
		return (T*)alloc();
	}
	
	template<typename T>
	T& get(int index) {
		return *(T*)(mem + index * sizeof(T));
	}
	
	template<typename T>
	T* type_addr(int offset) {
		return (T*)(mem + offset);
	}
	
	template<typename T>
	T& type_at(int offset) {
		return *type_addr<T>(offset);
	}
	
	static size_t get_num_alloc() {
		return num_alloc;
	}
	
	static void cleanup() {
		sync.lock();
		Memory* page = begin;
		while(page) {
			Memory* tmp = page;
			page = page->next;
			delete tmp;
			num_alloc--;
		}
		if(num_alloc) {
			std::cout << "WARNING: " << num_alloc << " pages of size " << size << " still being used at exit." << std::endl;
		}
		begin = 0;
		sync.unlock();
	}
	
	char* mem;
	
	Memory* next;
	
private:
	Memory() : mem(0), next(0) {
		mem = (char*)::malloc(size);
		assert(mem != OUT_OF_MEMORY);
	}
	
	~Memory() {
		::free(mem);
	}
	
private:
	static const int OUT_OF_MEMORY = 0;
	static util::spinlock sync;
	static Memory* begin;
	static size_t num_alloc;
	
};

template<int size_> util::spinlock Memory<size_>::sync;
template<int size_> Memory<size_>* Memory<size_>::begin = 0;
template<int size_> size_t Memory<size_>::num_alloc = 0;

typedef Memory<VNL_PAGE_SIZE> Page;
typedef Memory<VNL_BLOCK_SIZE> Block;


template<typename TPage>
class Allocator {
public:
	Allocator() {}
	
	~Allocator() {
		clear();
	}
	
	Allocator(const Allocator&) = delete;
	Allocator& operator=(const Allocator&) = delete;
	
	template<typename T>
	T* create() {
		return new(alloc<T>()) T();
	}
	
	template<typename T>
	void* alloc() {
		return alloc(sizeof(T));
	}
	
	void* alloc(int size) {
		assert(size <= TPage::size);
		if(!p_back) {
			p_front = TPage::alloc();
			p_back = p_front;
		}
		if(TPage::size - pos < size) {
			if(!p_back->next) {
				p_back->next = TPage::alloc();
			}
			p_back = p_back->next;
			pos = 0;
		}
		void* ptr = p_back->mem + pos;
		pos += size;
		return ptr;
	}
	
	TPage* front() {
		return p_front;
	}
	
	void clear() {
		if(p_front) {
			p_front->free_all();
			p_front = 0;
			p_back = 0;
			pos = 0;
		}
	}
	
private:
	TPage* p_front = 0;
	TPage* p_back = 0;
	int pos = 0;
	
};

typedef Allocator<Page> PageAllocator;
typedef Allocator<Block> BlockAllocator;


}

#endif /* INCLUDE_PHY_MEMORY_H_ */
