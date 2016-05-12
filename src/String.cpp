/*
 * String.cpp
 *
 *  Created on: May 8, 2016
 *      Author: mad
 */

#include "String.h"
#include "util/spinlock.h"


namespace vnl {

std::atomic<String::chunk_t*> String::begin;
phy::Region* String::memory = 0;


void String::write(const char* str, size_t len) {
	size_t pos = 0;
	chunk_t* buf = p_back;
	while(len > pos) {
		if(!buf || buf->len == CHUNK_SIZE) {
			buf = alloc();
			push_back(buf);
		}
		size_t num = len - pos;
		int left = CHUNK_SIZE - buf->len;
		if(num > left) { num = left; }
		memcpy(buf->str + buf->len, str+pos, num);
		buf->len += num;
		pos += num;
	}
}

void String::push_back(chunk_t* chunk) {
	chunk->next = 0;
	if(p_back) {
		p_back->next = chunk;
		p_back = chunk;
	} else {
		p_back = chunk;
		p_front = chunk;
	}
}

void String::clear() {
	chunk_t* chunk = p_front;
	while(chunk) {
		chunk_t* next = chunk->next;
		free(chunk);
		chunk = next;
	}
	p_front = 0;
	p_back = 0;
}

String::chunk_t* String::alloc() {
	static vnl::util::spinlock mutex;
	mutex.lock();
	chunk_t* chunk = begin;
	if(chunk) {
		while(!begin.compare_exchange_weak(chunk, chunk->next)) {
			if(!chunk) {
				chunk = memory->create<chunk_t>();
				break;
			}
		}
		chunk->next = 0;
		chunk->len = 0;
	} else {
		chunk = memory->create<chunk_t>();
	}
	mutex.unlock();
	assert(chunk != 0);
	return chunk;
}

void String::free(chunk_t* chunk) {
	chunk->next = begin;
	while(!begin.compare_exchange_weak(chunk->next, chunk)) {}
}




} // vnl

