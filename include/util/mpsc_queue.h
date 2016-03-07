/*
 * queue.h
 *
 *  Created on: Mar 5, 2016
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_MPSC_QUEUE_H_
#define INCLUDE_UTIL_MPSC_QUEUE_H_

#include <atomic>
#include <mutex>

namespace vnl { namespace util {

template<typename T, int N = 20>
class mpsc_queue {
public:
	mpsc_queue() : avail(0) {
		front = new block_t();
		write = front;
		back = front;
	}
	
	~mpsc_queue() {
		block_t* block = front;
		while(block) {
			block_t* tmp = block->next;
			delete block;
			block = tmp;
		}
	}
	
	void push(T obj) {
		block_t* block = write.load(std::memory_order_acquire);
		int index;
		while(true) {
			index = block->write.fetch_add(1, std::memory_order_acq_rel);
			if(index >= N) {
				block_t* next = block->next.load(std::memory_order_acquire);
				if(next == 0) {
					push_node(new block_t());
					do {
						next = block->next.load(std::memory_order_acquire);
					} while(next == 0);
				}
				write.compare_exchange_strong(block, next, std::memory_order_acq_rel);
			} else {
				break;
			}
		}
		block->elem[index] = obj;
		avail.fetch_add(1, std::memory_order_acq_rel);
	}
	
	bool pop(T& ref) {
		if(avail.load(std::memory_order_acquire)) {
			if(front->read >= N) {
				block_t* tmp = front;
				tmp->read = 0;
				tmp->write.store(0);
				front = front->next.exchange(0, std::memory_order_relaxed);
				push_node(tmp);
			}
			ref = front->elem[front->read++];
			avail.fetch_sub(1, std::memory_order_acq_rel);
			return true;
		}
		return false;
	}
	
private:
	struct block_t {
		block_t() : read(0), write(0), next(0) {}
		T elem[N];
		int read;
		std::atomic<int> write;
		std::atomic<block_t*> next;
	};
	
	void push_node(block_t* next) {
		block_t* node = back.exchange(next, std::memory_order_acq_rel);
		node->next.store(next, std::memory_order_release);
	}
	
private:
	block_t* front;
	std::atomic<block_t*> write;
	std::atomic<block_t*> back;
	std::atomic<int> avail;
	
};


}}

#endif /* INCLUDE_UTIL_MPSC_QUEUE_H_ */
