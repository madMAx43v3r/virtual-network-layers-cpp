/*
 * Queue.h
 *
 *  Created on: Mar 5, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_QUEUE_H_
#define INCLUDE_PHY_QUEUE_H_

#include "vnl/Memory.h"

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
		Block* block = p_front;
		while(block) {
			Block* next = block->next;
			block->free();
			block = next;
		}
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
		if(!p_front) {
			p_front = Block::alloc_ex<block_t>()->create();
			p_back = p_front;
		}
		if(p_back->write() >= N) {
			if(!p_back->next_block()) {
				p_back->next_block() = Block::alloc_ex<block_t>()->create();
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
	
	T& operator[](size_t index) {
		auto iter = begin();
		for(size_t i = 0; i < index; ++i) {
			++iter;
		}
		return *iter;
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
	
	size_t size() const {
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
		block_t* create() {
			read() = 0;
			write() = 0;
			return this;
		}
		block_t*& next_block() { return *((block_t**)(&next)); }
		int16_t& read() { return type_at<int16_t>(0); }
		int16_t& write() { return type_at<int16_t>(2); }
		T& elem(int i) { return type_at<T>(4 + i*sizeof(T)); }
	};
	
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
			return block->elem(pos);
		}
		typename std::iterator<std::forward_iterator_tag, P>::pointer operator->() const {
			return &block->elem(pos);
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
			:	block(block), pos(pos)
		{
			if(pos >= N) {
				this->block = block->next_block();
				this->pos = 0;
			}
		}
		void advance() {
			if(pos >= N-1) {
				block = block->next_block();
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
	
	iterator begin() { return iterator(p_front, p_front ? p_front->read() : 0); }
	const_iterator begin() const { return const_iterator(p_front, p_front ? p_front->read() : 0); }
	const_iterator cbegin() const { return const_iterator(p_front, p_front ? p_front->read() : 0); }
	
	iterator end() { return iterator(p_back, p_back ? p_back->write() : 0); }
	const_iterator end() const { return const_iterator(p_back, p_back ? p_back->write() : 0); }
	const_iterator cend() const { return const_iterator(p_back, p_back ? p_back->write() : 0); }
	
private:
	block_t* p_front = 0;
	block_t* p_back = 0;
	size_t count = 0;
	
};


}

#endif /* INCLUDE_PHY_QUEUE_H_ */
