/*
 * lockfree_spsc_queue.h
 *
 *  Created on: Dec 21, 2015
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_LOCKFREE_SPSC_QUEUE_H_
#define INCLUDE_UTIL_LOCKFREE_SPSC_QUEUE_H_

#include <atomic>

namespace vnl { namespace util {

template<typename T>
class lockfree_spsc_queue {
public:
	lockfree_spsc_queue() : avail(0) {
		front = new node_t();
		write = front;
		back = front;
	}
	
	~lockfree_spsc_queue() {
		node_t* node = front;
		while(node) {
			node_t* next = node->next;
			delete node;
			node = next;
		}
	}
	
	void push(const T& obj) {
		node_t* node = write;
		if(write == back.load(std::memory_order_acquire)) {
			push_node(new node_t());
		}
		node_t* next;
		do {
			next = write->next.load(std::memory_order_acquire);
		} while(next == 0);
		write = next;
		node->obj = obj;
		avail.fetch_add(1, std::memory_order_acq_rel);
	}
	
	bool pop(T& obj) {
		if(avail.load(std::memory_order_acquire) > 0) {
			obj = front->obj;
			node_t* next = front->next.exchange(0, std::memory_order_relaxed);
			push_node(front);
			front = next;
			avail.fetch_sub(1, std::memory_order_acq_rel);
			return true;
		} else {
			return false;
		}
	}
	
protected:
	struct node_t {
		node_t() : next(0) {}
		std::atomic<node_t*> next;
		T obj;
	};
	
	void push_node(node_t* next) {
		node_t* node = back.exchange(next, std::memory_order_acq_rel);
		node->next.store(next, std::memory_order_release);
	}
	
private:
	node_t* front;
	std::atomic<size_t> avail;
	std::atomic<node_t*> back;
	node_t* write;
	
};

}}

#endif /* INCLUDE_UTIL_LOCKFREE_SPSC_QUEUE_H_ */
