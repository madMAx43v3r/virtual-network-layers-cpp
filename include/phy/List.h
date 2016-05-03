/*
 * List.h
 *
 *  Created on: May 2, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_LIST_H_
#define INCLUDE_PHY_LIST_H_

#include "phy/Memory.h"

namespace vnl { namespace phy {

/*
 * This is a list.
 * Maximum element size is 4000 bytes.
 */
template<typename T>
class List {
public:
	List(Region* mem) : mem(mem) {
		p_front = 0;
		p_back = 0;
	}
	
protected:
	struct entry_t {
		T elem;
		entry_t* next = 0;
	};
	
public:
	T& push(T obj) {
		if(!p_front) {
			p_front = mem->create<entry_t>();
			p_back = p_front;
		} else {
			if(!p_back->next) {
				p_back->next = mem->create<entry_t>();
			}
			p_back = p_back->next;
		}
		T& ref = &p_back->elem;
		ref = obj;
		return ref;
	}
	
	T pop() {
		entry_t* tmp = p_front;
		p_front = p_front->next;
		if(p_back != tmp) {
			tmp->next = p_back->next;
			p_back->next = tmp;
		}
		return tmp->elem;
	}
	
	bool pop(T& ref) {
		if(!empty()) {
			ref = pop();
			return true;
		}
		return false;
	}
	
	T& front() {
		return p_front->elem;
	}
	const T& front() const {
		return p_front->elem;
	}
	
	T& back() {
		return p_back->elem;
	}
	const T& back() const {
		return p_back->elem;
	}
	
	bool empty() const {
		return p_front == 0;
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
			return current->elem;
		}
		typename std::iterator<std::forward_iterator_tag, P>::pointer operator->() const {
			return &current->elem;
		}
		friend void swap(iterator_t& lhs, iterator_t& rhs) {
			std::swap(lhs.current, rhs.current);
		}
		friend bool operator==(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.current == rhs.current;
		}
		friend bool operator!=(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.current != rhs.current;
		}
	private:
		iterator_t(entry_t* entry)
			:	current(entry) {}
		void advance() {
			current = current->next;
		}
		entry* current;
		friend class Queue;
	};
	
	typedef iterator_t<T> iterator;
	typedef iterator_t<const T> const_iterator;
	
	iterator begin() { return iterator(p_front); }
	const_iterator begin() const { return const_iterator(p_front); }
	const_iterator cbegin() const { return const_iterator(p_front); }
	
	iterator end() { return iterator(p_back->next); }
	const_iterator end() const { return const_iterator(p_back->next); }
	const_iterator cend() const { return const_iterator(p_back->next); }
	
protected:
	Region* mem;
	entry_t* p_front;
	entry_t* p_back;
	
	
};




}}


#endif /* INCLUDE_PHY_LIST_H_ */
