/*
 * List.h
 *
 *  Created on: May 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ARRAY_H_
#define INCLUDE_PHY_ARRAY_H_

#include <vector>

#include <vnl/Memory.h>


namespace vnl {

/*
 * This is a paged array.
 * Maximum element size is VNL_PAGE_SIZE bytes.
 */
template<typename T, typename TPage = Memory<VNL_PAGE_SIZE> >
class Array {
public:
	Array() : p_front(0), p_back(0), pos(0) {
		assert(sizeof(T) <= TPage::size);
	}
	
	Array(const Array& other) : p_front(0), p_back(0), pos(0) {
		append(other);
	}
	
	~Array() {
		clear();
	}
	
	Array& operator=(const Array& other) {
		if(&other != this) {
			clear();
			append(other);
		}
		return *this;
	}
	
	void append(const Array& other) {
		for(const_iterator iter = other.begin(); iter != other.end(); ++iter) {
			push_back(*iter);
		}
	}
	
	bool operator!=(const Array& other) const {
		return !(*this == other);
	}
	
	bool operator==(const Array& other) const {
		if(size() == other.size()) {
			const_iterator cmp = other.begin();
			for(const_iterator it = begin(); it != end(); ++it, ++cmp) {
				if(*it != *cmp) {
					return false;
				}
			}
			return true;
		}
		return false;
	}
	
	T& push_back() {
		return push_back(T());
	}
	
	T& push_back(const T& obj) {
		if(!p_front) {
			p_front = TPage::alloc();
			p_back = p_front;
		}
		if(pos >= M) {
			p_back->next = TPage::alloc();
			p_back = p_back->next;
			pos = 0;
		}
		T& ref = p_back->template type_at_index<T>(pos++);
		new(&ref) T();
		ref = obj;
		return ref;
	}
	
	Array& operator=(const std::vector<T>& vec) {
		clear();
		for(const T& elem : vec) {
			push_back(elem);
		}
		return *this;
	}
	
	T& operator[](int index) {
		int pi = index / M;
		int ei = index % M;
		TPage* page = p_front;
		for(int i = 0; i < pi; ++i) {
			page = page->next;
		}
		return page->template type_at_index<T>(ei);
	}
	
	std::vector<T> to_vector() const {
		int n = size();
		std::vector<T> vec(n);
		int i = 0;
		for(const_iterator iter = begin(); iter != end(); ++iter) {
			vec[i] = *iter;
			i++;
		}
		return vec;
	}
	
	void clear() {
		if(p_front) {
			for(iterator iter = begin(); iter != end(); ++iter) {
				iter->~T();
			}
			p_front->free_all();
			p_front = 0;
			p_back = 0;
			pos = 0;
		}
	}
	
	int size() const {
		int count = 0;
		TPage* page = p_front;
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
		return *begin();
	}
	T& back() {
		return (*this)[size()-1];
	}
	
	const T& front() const {
		return *begin();
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
		iterator_t() : page(0), pos(0) {}
		iterator_t(const iterator_t& other) : page(other.page), pos(other.pos) {}
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
			return page->template type_at_index<P>(pos);
		}
		typename std::iterator<std::forward_iterator_tag, P>::pointer operator->() const {
			return &page->template type_at_index<P>(pos);
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
		iterator_t(TPage* page, int pos)
			:	page(page), pos(pos) {}
		void advance() {
			if(pos >= Array::M-1 && page->next) {
				page = page->next;
				pos = 0;
			} else {
				pos++;
			}
		}
		TPage* page;
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
	static const int M = TPage::size / sizeof(T);
	
	TPage* p_front;
	TPage* p_back;
	int pos;
	
};


} // vnl

#endif /* INCLUDE_PHY_ARRAY_H_ */
