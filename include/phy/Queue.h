/*
 * Queue.h
 *
 *  Created on: Mar 5, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_QUEUE_H_
#define INCLUDE_PHY_QUEUE_H_

#include "phy/Memory.h"

namespace vnl { namespace phy {

template<typename T, int N = 20>
class Queue {
public:
	Queue(Region* mem) : mem(mem) {
		p_front = mem->create<block_t>();
		p_back = p_front;
	}
	
protected:
	struct block_t {
		T elem[N];
		int read = 0;
		int write = 0;
		block_t* next = 0;
	};
	
public:
	T* push(T obj) {
		if(p_back->write >= N) {
			if(!p_back->next) {
				p_back->next = mem->create<block_t>();
			}
			p_back = p_back->next;
		}
		T* ptr = &p_back->elem[p_back->write++];
		*ptr = obj;
		return ptr;
	}
	
	T pop() {
		if(p_front->read >= N) {
			block_t* tmp = p_front;
			tmp->read = 0;
			tmp->write = 0;
			p_front = p_front->next;
			tmp->next = p_back->next;
			p_back->next = tmp;
		}
		return p_front->elem[p_front->read++];
	}
	
	bool pop(T& ref) {
		if(!empty()) {
			ref = pop();
			return true;
		}
		return false;
	}
	
	T& front() {
		return p_front->elem[p_front->read];
	}
	
	T& back() {
		return p_back->elem[p_back->write - 1];
	}
	
	bool empty() {
		return p_front->read == p_front->write && p_front == p_back;
	}
	
private:
	Region* mem;
	block_t* p_front;
	block_t* p_back;
	
};


}}

#endif /* INCLUDE_PHY_QUEUE_H_ */
