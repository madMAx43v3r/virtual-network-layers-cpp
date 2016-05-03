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
		assert(Engine::local);
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
	
	~Page() {
		::free(mem);
	}
	
	friend class Engine;
	
};


class Region {
public:
	Region(Engine* engine) : engine(engine) {
		assert(engine);
		p_front = engine->get_page();
		p_back = p_front;
		left = Page::size;
	}
	
	Region() : Region(Engine::local) {}
	
	Region(const Region&) = delete;
	Region& operator=(const Region&) = delete;
	
	~Region() {
		p_front->free_all();
	}
	
	Page* get_page() {
		return engine->get_page();
	}
	
	void free_page(Page* page) {
		engine->free_page(page);
	}
	
	void* alloc(int size) {
		assert(size <= Page::size);
		if(left < size) {
			p_back->next = engine->get_page();
			p_back = p_back->next;
			left = Page::size;
		}
		void* ptr = p_back->mem + (Page::size - left);
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
	
	template<typename T>
	void destroy(T* obj) {
		obj->~T();
	}
	
private:
	Engine* engine;
	Page* p_front;
	Page* p_back;
	int left;
	
};



}}

#endif /* INCLUDE_PHY_MEMORY_H_ */
