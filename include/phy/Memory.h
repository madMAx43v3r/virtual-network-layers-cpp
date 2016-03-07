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


}}

#endif /* INCLUDE_PHY_MEMORY_H_ */
