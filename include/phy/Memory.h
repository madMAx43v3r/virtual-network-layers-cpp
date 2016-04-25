/*
 * Page.h
 *
 *  Created on: Mar 5, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_MEMORY_H_
#define INCLUDE_PHY_MEMORY_H_

#include "phy/Engine.h"

namespace vnl { namespace phy {

class Page {
public:
	static const int size = 4096;
	
	static Page* alloc() {
		return Engine::local->get_page();
	}
	
	Page(const Page&) = delete;
	Page& operator=(const Page&) = delete;
	
	void free() {
		engine->free_page(this);
	}
	
	void free_all() {
		if(next) {
			next->free();
		}
		free();
	}
	
	char* mem;
	
	Page* next;
	
private:
	Engine* engine;
	
	Page(Engine* engine) : next(0), engine(engine) {
		mem = (char*)malloc(size);
	}
	
	friend class Engine;
	
};


class Region {
public:
	Region(Engine* engine) : engine(engine) {
		assert(engine);
		begin = engine->get_page();
		page = begin;
		left = Page::size;
	}
	
	Region() : Region(Engine::local) {}
	
	~Region() {
		begin->free_all();
	}
	
	void* alloc(int size) {
		assert(size <= 1024);
		if(left < size) {
			page->next = engine->get_page();
			page = page->next;
			left = Page::size;
		}
		void* ptr = page->mem + (Page::size - left);
		left -= size;
		return ptr;
	}
	
	template<typename T>
	void* alloc() {
		return alloc(sizeof(T));
	}
	
	template<typename T>
	T* create() {
		return new(alloc<T>()) T();
	}
	
private:
	Engine* engine;
	Page* begin;
	Page* page;
	int left;
	
};


}}

#endif /* INCLUDE_PHY_MEMORY_H_ */
