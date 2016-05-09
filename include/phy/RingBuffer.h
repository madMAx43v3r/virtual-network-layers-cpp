/*
 * RingBuffer.h
 *
 *  Created on: Apr 25, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_RINGBUFFER_H_
#define INCLUDE_PHY_RINGBUFFER_H_

#include "Queue.h"


namespace vnl { namespace phy {

class RingBuffer {
public:
	RingBuffer(Region& mem) : queue(mem) {
		p_front = Page::alloc();
		p_back = p_front;
		pos = 0;
	}
	
	RingBuffer(const RingBuffer&) = delete;
	RingBuffer& operator=(const RingBuffer&) = delete;
	
	~RingBuffer() {
		p_front->free_all();
	}
	
	struct entry_t {
		Page* page;
		void* ptr;
		bool free = false;
	};
	
	template<typename T>
	T* create(entry_t*& p_entry) {
		p_entry = alloc<T>();
		return new(p_entry->ptr) T();
	}
	
	template<typename T>
	void destroy(entry_t* entry) {
		((T*)entry->ptr)->~T();
		free(entry);
	}
	
	template<typename T>
	entry_t* alloc() {
		return alloc(sizeof(T));
	}
	
	entry_t* alloc(int size) {
		assert(size <= Page::size);
		if(Page::size - pos < size) {
			p_back->next = Page::alloc();
			p_back = p_back->next;
			pos = 0;
		}
		entry_t entry;
		entry.page = p_back;
		entry.ptr = p_back->mem + pos;
		pos += size;
		return &queue.push(entry);
	}
	
	void free(entry_t* entry) {
		entry->free = true;
		while(!queue.empty()) {
			entry_t& front = queue.front();
			if(front.page != p_front) {
				p_front->free();
				p_front = front.page;
			}
			if(!front.free) {
				break;
			}
			queue.pop();
		}
	}
	
private:
	Queue<entry_t> queue;
	Page* p_front;
	Page* p_back;
	int pos;
	
};


}}

#endif /* INCLUDE_PHY_RINGBUFFER_H_ */
