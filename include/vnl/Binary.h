/*
 * Binary.h
 *
 *  Created on: Jul 4, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_BINARY_H_
#define CPP_INCLUDE_VNI_BINARY_H_

#include <vnl/Memory.h>


namespace vnl {

class Binary {
public:
	vnl::Page* data;
	int size;
	
	Binary() : data(0), size(0) {
	}
	
	Binary(const Binary& other) : data(0), size(0) {
		*this = other;
	}
	
	~Binary() {
		clear();
	}
	
	Binary& operator=(const Binary& other) {
		clear();
		data = other.data ? vnl::Page::alloc() : 0;
		size = other.size;
		vnl::Page* src = other.data;
		vnl::Page* dst = data;
		while(src) {
			::memcpy(dst->mem, src->mem, vnl::Page::size);
			if(src->next) {
				dst->next = vnl::Page::alloc();
			}
			src = src->next;
			dst = dst->next;
		}
		return *this;
	}
	
	void clear() {
		if(data) {
			data->free_all();
			data = 0;
		}
		size = 0;
	}
	
};


}


#endif /* CPP_INCLUDE_VNI_BINARY_H_ */
