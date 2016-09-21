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
		T tmp;
		while(pop(tmp));
		Block* block = p_front;
		while(block) {
			Block* next = block->next;
			block->free();
			block = next;
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
			if(!p_back->next_block()) {
				p_back->next_block() = block_t::create();
			}
			p_back = p_back->next_block();
			p_back->write() = 0;
		}
		T& ref = p_back->elem(p_back->write()++);
		new (&ref) T();
		ref = obj;
		count++;
		return ref;
	}
	
	bool pop(T& obj) {
		if(empty()) {
			return false;
		}
		T& tmp = p_front->elem(p_front->read()++);
		if(p_front->read() >= N) {
			if(p_front != p_back) {
				block_t* tmp = p_front;
				p_front = p_front->next_block();
				tmp->next_block() = p_back->next_block();
				p_back->next_block() = tmp;
			} else {
				p_front->write() = 0;
			}
			p_front->read() = 0;
		}
		obj = tmp;
		tmp.~T();
		count--;
		return true;
	}
	
	T pop() {
		T obj;
		pop(obj);
		return obj;
	}
	
	T& front() {
		return p_front->elem(p_front->read());
	}
	T& back() {
		return p_back->elem(p_back->write() - 1);
	}
	
	const T& front() const {
		return p_front->elem(p_front->read());
	}
	const T& back() const {
		return p_back->elem(p_back->write() - 1);
	}
	
	int size() const {
		return count;
	}
	
	void clear() {
		if(p_front) {
			p_front->read() = 0;
			p_front->write() = 0;
		}
		p_back = p_front;
		count = 0;
	}
	
	bool empty() const {
		return count == 0;
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
	int count = 0;
	
};


}

#endif /* INCLUDE_PHY_QUEUE_H_ */
