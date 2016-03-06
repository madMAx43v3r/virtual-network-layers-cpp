/*
 * queue.h
 *
 *  Created on: Mar 5, 2016
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_QUEUE_H_
#define INCLUDE_UTIL_QUEUE_H_

#include "phy/Memory.h"

namespace vnl { namespace phy {

template<typename T>
class Queue {
public:
	Queue() {
		front = new block_t();
		back = front;
	}
	
	~Queue() {
		block_t* block = front;
		while(block) {
			block_t* tmp = block->next;
			delete block;
			block = tmp;
		}
	}
	
protected:
	struct block_t {
		static const int size = 30;
		T elem[size];
		int read = 0;
		int write = 0;
		block_t* next = 0;
	};
	
public:
	void push(T obj) {
		if(back->write >= block_t::size) {
			if(!back->next) {
				back->next = new block_t();
			}
			back = back->next;
		}
		back->elem[back->write++] = obj;
	}
	
	T pop() {
		if(front->read >= block_t::size) {
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
		return front->read == back->write && front == back;
	}
	
private:
	block_t* front;
	block_t* back;
	
};


}}

#endif /* INCLUDE_UTIL_QUEUE_H_ */
