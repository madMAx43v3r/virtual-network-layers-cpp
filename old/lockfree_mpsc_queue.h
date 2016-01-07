/*
 * lockfree_mpsc_queue.h
 *
 *  Created on: Dec 20, 2015
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_LOCKFREE_MPSC_QUEUE_H_
#define INCLUDE_UTIL_LOCKFREE_MPSC_QUEUE_H_

#include <atomic>

namespace vnl { namespace util {

template<typename T>
class lockfree_mpsc_queue {
public:
	lockfree_mpsc_queue() : avail(0) {
		front = new node_t();
		write = front;
		back = front;
		stat_reset();
	}
	
	~lockfree_mpsc_queue() {
		node_t* node = front;
		while(node) {
			node_t* next = node->next;
			delete node;
			node = next;
		}
	}
	
	void push(const T& obj) {
		node_t* node = 0;
		while(true) {
			node = write.load(std::memory_order_acquire);
			node_t* next = node->next.load(std::memory_order_acquire);
			if(next == 0) {
				push_node(new node_t());
				do {
					next = node->next.load(std::memory_order_acquire);
				} while(next == 0);
			}
			if(write.compare_exchange_strong(node, next, std::memory_order_acq_rel)) {
				break;
			}
			stat_push_fail++;
		}
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
			stat_passed++;
			return true;
		} else {
			stat_pop_fail++;
			return false;
		}
	}
	
	void stat_reset() {
		stat_passed = 0;
		stat_push_fail = 0;
		stat_pop_fail = 0;
	}
	
	int stat_passed;
	int stat_pop_fail;
	int stat_push_fail;
	
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
	std::atomic<size_t> avail;
	std::atomic<node_t*> write;
	std::atomic<node_t*> back;
	node_t* front;
	
	
	
};

}}

#endif /* INCLUDE_UTIL_LOCKFREE_MPSC_QUEUE_H_ */
