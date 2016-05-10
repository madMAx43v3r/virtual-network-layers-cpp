/*
 * String.cpp
 *
 *  Created on: May 8, 2016
 *      Author: mad
 */

#include "String.h"


namespace vnl {

phy::AtomicPool<String::chunk_t>* String::chunks = 0;


void String::write(const char* str, size_t len) {
		size_t pos = 0;
		chunk_t* buf = p_back;
		while(len > pos) {
			if(!buf || buf->len == CHUNK_SIZE) {
				buf = chunks->create();
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
		chunks->destroy(chunk);
		chunk = next;
	}
	p_front = 0;
	p_back = 0;
}



} // vnl

