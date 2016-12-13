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
	Page* data;
	int size;
	
	Binary() : data(0), size(0) {}
	
	Binary(const Binary& other) : data(0), size(0) {
		*this = other;
	}
	
	Binary(const void* in_data, int in_size) : data(0), size(0) {
		if(in_size > 0) {
			data = Page::alloc();
			Page* curr = data;
			while(true) {
				int n = in_size > Page::size ? Page::size : in_size;
				::memcpy(curr->mem, (const char*)in_data + size, n);
				in_size -= n;
				size += n;
				if(in_size > 0) {
					curr->next = Page::alloc();
					curr = curr->next;
				} else {
					break;
				}
			}
		}
	}
	
	template<typename T>
	Binary(const T& value) {
		data = Page::alloc();
		vnl::io::ByteBuffer buf(data);
		vnl::io::TypeOutput out(&buf);
		vnl::write(out, value);
		out.flush();
		size = buf.position();
	}
	
	~Binary() {
		clear();
	}
	
	Binary& operator=(const Binary& other) {
		clear();
		if(other.data && other.size > 0) {
			data = Page::alloc();
			size = other.size;
			Page* src = other.data;
			Page* dst = data;
			while(src) {
				::memcpy(dst->mem, src->mem, Page::size);
				if(src->next) {
					dst->next = Page::alloc();
				}
				src = src->next;
				dst = dst->next;
			}
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


} // vnl

#endif /* CPP_INCLUDE_VNI_BINARY_H_ */
