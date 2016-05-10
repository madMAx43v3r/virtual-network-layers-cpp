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
	Page* page = begin;
	if(page) {
		while(!begin.compare_exchange_weak(page, page->next)) {
			if(!page) {
				page = new Page();
				break;
			}
		}
		page->next = 0;
	} else {
		page = new Page();
	}
	assert(page != OUT_OF_MEMORY);
	return page;
}

void Page::free() {
	next = begin;
	while(!begin.compare_exchange_weak(next, this)) {}
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

