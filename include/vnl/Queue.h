/*
 * Queue.h
 *
 *  Created on: Mar 5, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_QUEUE_H_
#define INCLUDE_PHY_QUEUE_H_

#include <vnl/Memory.h>

namespace vnl {

/*
 * This is a queue for use on small data types.
 */
template<typename T>
class Queue {
public:
	static const int N = (VNL_BLOCK_SIZE - 4) / sizeof(T);
	
	Queue() {
		assert(N > 0);
	}
	
	~Queue() {
		clear();
		if(p_front) {
			p_front->free();
		}
	}
	
	Queue(const Queue& other) = delete;
	Queue& operator=(const Queue& other) = delete;
	
	T& push() {
		return push(T());
	}
	
	T& push(const T& obj) {
		if(!p_front) {
			p_front = block_t::create();
			p_back = p_front;
		}
		if(p_back->write() >= N) {
			block_t* next = block_t::create();
			p_back->next_block() = next;
			p_back = next;
		}
		T& ref = p_back->elem(p_back->write()++);
		new(&ref) T();
		ref = obj;
		count++;
		return ref;
	}
	
	bool pop(T& obj) {
		if(!count) {
			return false;
		}
		T& tmp = p_front->elem(p_front->read()++);
		obj = tmp;
		tmp.~T();
		if(p_front->read() >= N) {
			block_t* old = p_front;
			if(p_front == p_back) {
				p_front = 0;
				p_back = 0;
			} else {
				p_front = p_front->next_block();
			}
			old->free();
		}
		count--;
		return true;
	}
	
	T pop() {
		T obj;
		if(!pop(obj)) {
			raise_null_pointer();
		}
		return obj;
	}
	
	T& front() {
		if(!count) {
			raise_null_pointer();
		}
		return p_front->elem(p_front->read());
	}
	T& back() {
		if(!count) {
			raise_null_pointer();
		}
		return p_back->elem(p_back->write() - 1);
	}
	
	const T& front() const {
		if(!count) {
			raise_null_pointer();
		}
		return p_front->elem(p_front->read());
	}
	const T& back() const {
		if(!count) {
			raise_null_pointer();
		}
		return p_back->elem(p_back->write() - 1);
	}
	
	size_t size() const {
		return count;
	}
	
	bool empty() const {
		return count == 0;
	}
	
	void clear() {
		T tmp;
		while(pop(tmp));
	}
	
protected:
	class block_t : public Block {
	public:
		static block_t* create() {
			block_t* block = Block::alloc_ex<block_t>();
			block->read() = 0;
			block->write() = 0;
			return block;
		}
		block_t*& next_block() { return *((block_t**)(&next)); }
		int16_t& read() { return type_at<int16_t>(0); }
		int16_t& write() { return type_at<int16_t>(2); }
		T& elem(int i) { return type_at<T>(4 + i*sizeof(T)); }
	};
	
private:
	block_t* p_front = 0;
	block_t* p_back = 0;
	size_t count = 0;
	
};


} // vnl

#endif /* INCLUDE_PHY_QUEUE_H_ */
