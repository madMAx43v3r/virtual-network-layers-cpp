/*
 * List.h
 *
 *  Created on: May 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ARRAY_H_
#define INCLUDE_PHY_ARRAY_H_

#include <vector>
#include <type_traits>
#include <initializer_list>

#include <vnl/Memory.h>


namespace vnl {

/*
 * This is a paged array.
 * Maximum element size is VNL_PAGE_SIZE bytes.
 * Warning: operator[] has O(n) complexity. Use Tree for O(log(n)).
 */
template<typename T, typename TPage = Memory<VNL_PAGE_SIZE> >
class Array {
public:
	Array() : p_front(0), p_back(0), pos(0), count(0) {
		assert(M > 0);
	}
	
	Array(const Array& other) : p_front(0), p_back(0), pos(0), count(0) {
		append(other);
	}

	Array(const T array[], int n) : p_front(0), p_back(0), pos(0), count(0) {
		append(array, n);
	}

	Array(std::initializer_list<T> list) : p_front(0), p_back(0), pos(0), count(0) {
		if(list.size() > 0) {
			for(const T* v=list.begin(); v!=list.end(); ++v) {
				push_back(*v);
			}
		}
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
	
	void append(const Array& other) {
		for(const_iterator iter = other.begin(); iter != other.end(); ++iter) {
			push_back(*iter);
		}
	}
	
	void append(const T array[], int n) {
		for(int i=0; i<n; i++) {
			push_back(array[i]);
		}
	}

	T& push_back() {
		return push_back(T());
	}
	
	T& push_back(const T& obj) {
		check();
		if(pos >= M) {
			extend();
		}
		count++;
		T& ref = p_back->template type_at_index<T>(pos++);
		new(&ref) T(obj);
		return ref;
	}

	T& operator[](int index) {
		assert(index >= 0 && index < count);
		int pi = index / M;
		int ei = index % M;
		TPage* page = p_front;
		for(int i = 0; i < pi; ++i) {
			page = page->next;
		}
		return page->template type_at_index<T>(ei);
	}

	const T& operator[](int index) const {
		assert(index >= 0 && index < count);
		int pi = index / M;
		int ei = index % M;
		TPage* page = p_front;
		for(int i = 0; i < pi; ++i) {
			page = page->next;
		}
		return page->template type_at_index<T>(ei);
	}
	
	Array& operator=(const std::vector<T>& vec) {
		clear();
		for(const T& elem : vec) {
			push_back(elem);
		}
		return *this;
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
	
	ssize_t to_array(T buf[], size_t len) const {
		int n = (size() < len) ? size() : len;
		for(size_t i=0; i<n; ++i) {
			buf[i] = (*this)[i];
		}
		return n;
	}

	void clear() {
		if(p_front) {
			if(std::is_class<T>::value) {
				for(iterator iter = begin(); iter != end(); ++iter) {
					iter->~T();
				}
			}
			p_front->free_all();
			p_front = 0;
			p_back = 0;
			pos = 0;
			count = 0;
		}
	}
	
	int size() const {
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
		return count == 0;
	}
	
protected:
	void check() {
		if(!p_front) {
			p_front = TPage::alloc();
			p_back = p_front;
		}
	}
	
	void extend() {
		p_back->next = TPage::alloc();
		p_back = p_back->next;
		pos = 0;
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
	
	Array(Array::const_iterator itbegin, Array::const_iterator itend) : Array() {
		for(Array::const_iterator it=itbegin; it!=itend; ++it) {
			push_back(*it);
		}
	}

protected:
	static const int M = TPage::size / sizeof(T);
	
	TPage* p_front;
	TPage* p_back;
	int pos;
	int count;
	
};


} // vnl

#endif /* INCLUDE_PHY_ARRAY_H_ */
