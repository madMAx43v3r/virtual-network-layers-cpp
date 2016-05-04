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

/*
 * This is a queue.
 * Maximum element size at default is 496 bytes.
 */
template<typename T, int N = 8>
class Queue {
public:
	Queue(Region* mem) : mem(mem) {
		p_front = mem->create<block_t>();
		p_back = p_front;
	}
	
	Queue(const Queue&) = delete;
	Queue& operator=(const Queue&) = delete;
	
protected:
	struct block_t {
		T elem[N];
		int read = 0;
		int write = 0;
		block_t* next = 0;
	};
	
public:
	T& push(const T& obj) {
		if(p_back->write >= N) {
			if(!p_back->next) {
				p_back->next = mem->create<block_t>();
			}
			p_back = p_back->next;
		}
		T& ref = &p_back->elem[p_back->write++];
		ref = obj;
		return ref;
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
	
	bool empty() const {
		return p_front->read == p_front->write && p_front == p_back;
	}
	
public:
	
	template<typename P>
	class iterator_t : public std::iterator<std::forward_iterator_tag, P> {
	public:
		iterator_t() : iterator_t(0) {}
		iterator_t(const iterator_t&) = default;
		iterator_t& operator++() {
			advance();
			return *this;
		}
		iterator_t operator++(int) {
			iterator_t tmp = *this;
			advance();
			return tmp;
		}
		typename std::iterator<std::forward_iterator_tag, P>::reference operator*() const {
			return block->elem[pos];
		}
		typename std::iterator<std::forward_iterator_tag, P>::pointer operator->() const {
			return &block->elem[pos];
		}
		friend void swap(iterator_t& lhs, iterator_t& rhs) {
			std::swap(lhs.block, rhs.block);
			std::swap(lhs.pos, rhs.pos);
		}
		friend bool operator==(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.block == rhs.block && lhs.pos == rhs.pos;
		}
		friend bool operator!=(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.block != rhs.block || lhs.pos != rhs.pos;
		}
	private:
		iterator_t(block_t* block, int pos)
			:	block(block), pos(pos) {}
		void advance() {
			if(pos >= N) {
				block = block->next;
				pos = 0;
			} else {
				pos++;
			}
		}
		block_t* block;
		int pos;
		friend class Queue;
	};
	
	typedef iterator_t<T> iterator;
	typedef iterator_t<const T> const_iterator;
	
	iterator begin() { return iterator(p_front, p_front->read); }
	const_iterator begin() const { return const_iterator(p_front, p_front->read); }
	const_iterator cbegin() const { return const_iterator(p_front, p_front->read); }
	
	iterator end() { return iterator(p_back, p_back->write); }
	const_iterator end() const { return const_iterator(p_back, p_back->write); }
	const_iterator cend() const { return const_iterator(p_back, p_back->write); }
	
protected:
	Region* mem;
	block_t* p_front;
	block_t* p_back;
	
	
};


}}

#endif /* INCLUDE_PHY_QUEUE_H_ */
