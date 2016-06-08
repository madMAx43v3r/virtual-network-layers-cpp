/*
 * List.h
 *
 *  Created on: May 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ARRAY_H_
#define INCLUDE_PHY_ARRAY_H_

#include "vnl/Memory.h"


namespace vnl {

/*
 * This is a paged array.
 * Maximum element size is 4096 bytes.
 */
template<typename T>
class Array {
public:
	Array() {
		assert(sizeof(T) <= Page::size);
	}
	
	Array(const Array& other) {
		append(other);
	}
	
	~Array() {
		clear();
	}
	
	Array& operator=(const Array& other) {
		clear();
		append(other);
		return *this;
	}
	
	void append(const Array& other) {
		for(auto iter = other.begin(); iter != other.end(); ++iter) {
			push_back(*iter);
		}
	}
	
	T& push_back(const T& obj) {
		if(!p_front) {
			p_front = Page::alloc();
			p_back = p_front;
		}
		if(pos >= M) {
			p_back->next = Page::alloc();
			p_back = p_back->next;
			pos = 0;
		}
		T& ref = p_back->get<T>(pos++);
		ref = obj;
		return ref;
	}
	
	T& operator[](size_t index) {
		int pi = index / M;
		int ei = index % M;
		Page* page = p_front;
		for(int i = 0; i < pi; ++i) {
			page = page->next;
		}
		return p_back->get<T>(ei);
	}
	
	void clear() {
		if(p_front) {
			for(auto iter = begin(); iter != end(); ++iter) {
				iter->~T();
			}
			p_front->free_all();
			p_front = 0;
			p_back = 0;
			pos = 0;
		}
	}
	
	size_t size() const {
		size_t count = 0;
		Page* page = p_front;
		while(page) {
			if(page != p_back) {
				count += M;
			} else {
				count += pos;
			}
			page = page->next;
		}
		return count;
	}
	
	T& front() {
		return (*this)[0];
	}
	T& back() {
		return (*this)[size()-1];
	}
	
	const T& front() const {
		return (*this)[0];
	}
	const T& back() const {
		return (*this)[size()-1];
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
			return page->get<P>(pos);
		}
		typename std::iterator<std::forward_iterator_tag, P>::pointer operator->() const {
			return &page->get<P>(pos);
		}
		friend void swap(iterator_t& lhs, iterator_t& rhs) {
			std::swap(lhs.page, rhs.page);
			std::swap(lhs.pos, rhs.pos);
		}
		friend bool operator==(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.page == rhs.page && lhs.pos == rhs.pos;
		}
		friend bool operator!=(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.page != rhs.page || lhs.pos != rhs.pos;
		}
	private:
		iterator_t(Page* page, int pos)
			:	page(page), pos(pos) {}
		void advance() {
			if(pos >= Array::M-1 && page->next) {
				page = page->next;
				pos = 0;
			} else {
				pos++;
			}
		}
		Page* page;
		int pos;
		friend class Array;
	};
	
	typedef iterator_t<T> iterator;
	typedef iterator_t<const T> const_iterator;
	
	iterator begin() { return iterator(p_front, 0); }
	const_iterator begin() const { return const_iterator(p_front, 0); }
	const_iterator cbegin() const { return const_iterator(p_front, 0); }
	
	iterator end() { return iterator(p_back, pos); }
	const_iterator end() const { return const_iterator(p_back, pos); }
	const_iterator cend() const { return const_iterator(p_back, pos); }
	
protected:
	static const int M = Page::size / sizeof(T);
	
	Page* p_front = 0;
	Page* p_back = 0;
	int pos = 0;
	
};



}

#endif /* INCLUDE_PHY_ARRAY_H_ */
