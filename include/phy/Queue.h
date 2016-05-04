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
 * Maximum element size at default is 500 bytes.
 */
template<typename T, int N = 8>
class Queue {
public:
	Queue(Region* mem) : mem(mem) {
		p_front = mem->create<block_t>();
		p_back = p_front;
	}
	
	Queue(const Queue&) = delete;
	
	Queue& operator=(const Queue& other) {
		clear();
		append(other);
		return *this;
	}
	
	void append(const Queue& other) {
		for(const T& obj : other) {
			push(obj);
		}
	}
	
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
	
	bool pop(T& obj) {
		if(!empty()) {
			if(p_front->read >= N) {
				block_t* tmp = p_front;
				tmp->read = 0;
				tmp->write = 0;
				p_front = p_front->next;
				tmp->next = p_back->next;
				p_back->next = tmp;
			}
			T& tmp = p_front->elem[p_front->read++];
			obj = tmp;
			tmp.~T();
			return true;
		}
		return false;
	}
	
	bool pop() {
		T obj;
		return pop(obj);
	}
	
	T& operator[](size_t index) {
		auto iter = begin();
		for(size_t i = 0; i < index; ++i) {
			++iter;
		}
		return *iter;
	}
	
	T& front() {
		return p_front->elem[p_front->read];
	}
	
	T& back() {
		return p_back->elem[p_back->write - 1];
	}
	
	size_t size() {
		size_t count = 0;
		for(auto iter = begin(); iter != end(); ++iter) {
			count++;
		}
		return count;
	}
	
	void clear() {
		for(auto iter = begin(); iter != end(); ++iter) {
			iter->~T();
		}
		p_front->read = 0;
		p_front->write = 0;
		p_back->read = 0;
		p_back->write = 0;
		p_back = p_front;
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
	struct block_t {
		T elem[N];
		block_t* next = 0;
		short read = 0;
		short write = 0;
	};
	
private:
	Region* mem;
	block_t* p_front;
	block_t* p_back;
	
	
};


}}

#endif /* INCLUDE_PHY_QUEUE_H_ */
