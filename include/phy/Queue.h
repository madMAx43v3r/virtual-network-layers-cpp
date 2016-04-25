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
		front = mem->create<block_t>();
		back = front;
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
				back->next = mem->create<block_t>();
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
	Region* mem;
	block_t* front;
	block_t* back;
	
};


}}

#endif /* INCLUDE_PHY_QUEUE_H_ */
