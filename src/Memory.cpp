/*
 * Memory.cpp
 *
 *  Created on: May 6, 2016
 *      Author: mad
 */

#include "vnl/Memory.h"


namespace vnl {

util::spinlock Page::mutex;

Page* Page::begin = 0;

size_t Page::num_alloc = 0;

Page* Page::alloc() {
	mutex.lock();
	Page* page = begin;
	if(page) {
		begin = page->next;
	} else {
		page = new Page();
		num_alloc++;
	}
	mutex.unlock();
	assert(page != OUT_OF_MEMORY);
	page->next = 0;
	return page;
}

void Page::free() {
	mutex.lock();
	next = begin;
	begin = this;
	mutex.unlock();
}

void Page::free_all() {
	Page* page = this;
	while(page) {
		Page* tmp = page;
		page = page->next;
		tmp->free();
	}
}

void Page::cleanup() {
	mutex.lock();
	Page* page = begin;
	while(page) {
		Page* tmp = page;
		page = page->next;
		delete tmp;
		num_alloc--;
	}
	if(num_alloc) {
		std::cout << "WARNING: " << num_alloc << " pages still being used at exit." << std::endl;
	}
	begin = 0;
	mutex.unlock();
}


void* Region::alloc(int size) {
	assert(size <= Page::size);
	if(!p_back) {
		p_front = Page::alloc();
		p_back = p_front;
	}
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

void Region::clear() {
	p_back = p_front;
	pos = 0;
}


}

