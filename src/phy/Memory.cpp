/*
 * Memory.cpp
 *
 *  Created on: May 6, 2016
 *      Author: mad
 */

#include "phy/Memory.h"


namespace vnl { namespace phy {

std::atomic<Page*> Page::begin;

size_t Page::num_alloc = 0;

Page* Page::alloc() {
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

void Page::free() {
	next = begin.load(std::memory_order_acquire);
	while(!begin.compare_exchange_strong(next, this, std::memory_order_acq_rel)) {}
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
	Page* page = begin;
	while(page) {
		Page* tmp = page;
		page = page->next;
		delete tmp;
	}
	if(num_alloc) {
		std::cout << "WARNING: " << num_alloc << " pages still being used at exit." << std::endl;
	}
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


}}

