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
		Page* page = Engine::local->get_page();
		page->next = 0;
		return page;
	}
	
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


template<typename T>
class Pool {
public:
	~Pool() {
		for(T* obj : list) {
			delete obj;
		}
	}
	
	T* alloc() {
		if(list.size()) {
			T* obj = list.back();
			list.pop_back();
			obj->~T();
			return new(obj) T();
		} else {
			return new T();
		}
	}
	
	void free(T* obj) {
		list.push_back(obj);
	}
	
protected:
	std::vector<T*> list;
	
};



}}

#endif /* INCLUDE_PHY_MEMORY_H_ */
