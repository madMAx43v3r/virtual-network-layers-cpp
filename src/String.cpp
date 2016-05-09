/*
 * String.cpp
 *
 *  Created on: May 8, 2016
 *      Author: mad
 */

#include "String.h"


namespace vnl {

phy::AtomicPool<String::chunk_t>* String::chunks = 0;


int String::overflow(int c) {
	if(buf) {
		buf->len = CHUNK_SIZE;
		push_back(buf);
	}
	buf = chunks->create();
	setp(buf->str, buf->str + CHUNK_SIZE);
	sputc(c);
	return c;
}

int String::sync() {
	if(buf) {
		buf->len = pptr() - pbase();
		push_back(buf);
		setp(0, 0);
		buf = 0;
	}
	return 0;
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
	if(buf) {
		chunks->destroy(buf);
		buf = 0;
	}
}



} // vnl

