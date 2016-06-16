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

#ifdef VNL_MEMORY_ALIGN
#define VNL_SIZE(size) (size + ((VNL_MEMORY_ALIGN - (size % VNL_MEMORY_ALIGN)) % VNL_MEMORY_ALIGN))
#define VNL_SIZEOF(type) VNL_SIZE(sizeof(type))
#else
#define VNL_SIZE(size) size
#define VNL_SIZEOF(type) sizeof(type)
#endif


namespace vnl {

template<int size_>
class Memory {
public:
	static const int size = size_;
	
	Memory(const Memory&) = delete;
	Memory& operator=(const Memory&) = delete;
	
	static Memory* alloc() {
		mutex.lock();
		Memory* page = begin;
		if(page) {
			begin = page->next;
		} else {
			page = new Memory();
			num_alloc++;
		}
		mutex.unlock();
		assert(page != OUT_OF_MEMORY);
		page->next = 0;
		return page;
	}
	
	void free() {
		mutex.lock();
		next = begin;
		begin = this;
		mutex.unlock();
	}
	
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
		return *(T*)(mem + index * VNL_SIZEOF(T));
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
		mutex.lock();
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
		mutex.unlock();
	}
	
	char* mem;
	
	Memory* next;
	
private:
	Memory() : mem(0), next(0) {
#ifdef VNL_MEMORY_ALIGN
		int err = ::posix_memalign((void**)&mem, VNL_MEMORY_ALIGN, size);
		assert(err == 0);
#else
		mem = (char*)::malloc(size);
#endif
		assert(mem != OUT_OF_MEMORY);
	}
	
	~Memory() {
		::free(mem);
	}
	
private:
	static const int OUT_OF_MEMORY = 0;
	static util::spinlock mutex;
	static Memory* begin;
	static size_t num_alloc;
	
};

template<int size_> util::spinlock Memory<size_>::mutex;
template<int size_> Memory<size_>* Memory<size_>::begin = 0;
template<int size_> size_t Memory<size_>::num_alloc = 0;

typedef Memory<VNL_PAGE_SIZE> Page;
typedef Memory<VNL_BLOCK_SIZE> Block;


template<typename TPage>
class Allocator {
public:
	Allocator() {}
	
	~Allocator() {
		free_all();
	}
	
	Allocator(const Allocator&) = delete;
	Allocator& operator=(const Allocator&) = delete;
	
	template<typename T>
	T* create() {
		return new(alloc<T>()) T();
	}
	
	template<typename T>
	void* alloc() {
#ifndef VNL_MEMORY_DEBUG
		return alloc(VNL_SIZEOF(T));
#else
		return malloc(sizeof(T));
#endif
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
		p_back = p_front;
		pos = 0;
	}
	
	void free_all() {
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

typedef Allocator<Page> PageAlloc;
typedef Allocator<Block> BlockAlloc;


}

#endif /* INCLUDE_PHY_MEMORY_H_ */
