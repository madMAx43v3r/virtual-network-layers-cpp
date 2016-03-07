/*
 * queue.h
 *
 *  Created on: Mar 5, 2016
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_QUEUE_H_
#define INCLUDE_UTIL_QUEUE_H_

#include "phy/Memory.h"

namespace vnl { namespace util {

template<typename T, int N = 20>
class queue {
public:
	queue() {
		front = new block_t();
		back = front;
	}
	
	~queue() {
		block_t* block = front;
		while(block) {
			block_t* tmp = block->next;
			delete block;
			block = tmp;
		}
	}
	
protected:
	struct block_t {
		T elem[N];
		int read = 0;
		int write = 0;
		block_t* next = 0;
	};
	
public:
	void push(T obj) {
		if(back->write >= N) {
			if(!back->next) {
				back->next = new block_t();
			}
			back = back->next;
		}
		back->elem[back->write++] = obj;
	}
	
	T pop() {
		if(front->read >= N) {
			block_t* tmp = front;
			tmp->read = 0;
			tmp->write = 0;
			front = front->next;
			tmp->next = back->next;
			back->next = tmp;
		}
		return front->elem[front->read++];
	}
	
	bool pop(T& ref) {
		if(!empty()) {
			ref = pop();
			return true;
		}
		return false;
	}
	
	bool empty() {
		return front->read == front->write && front == back;
	}
	
private:
	block_t* front;
	block_t* back;
	
};


}}

#endif /* INCLUDE_UTIL_QUEUE_H_ */
