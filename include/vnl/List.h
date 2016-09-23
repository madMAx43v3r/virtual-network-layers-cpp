/*
 * List.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_LIST_H_
#define INCLUDE_PHY_LIST_H_

#include <vnl/Memory.h>


namespace vnl {

/*
 * This is a doubly linked list for use on small data types.
 */
template<typename T>
class List {
public:
	List() : p_front(0), p_back(0), count(0) {}
	
	List(const List& other) : p_front(0), p_back(0), count(0) {
		append(other);
	}
	
	~List() {
		clear();
	}
	
protected:
	struct entry_t {
		entry_t* prev;
		entry_t* next;
		T value;
		entry_t() : prev(0), next(0) {}
	};
	
public:
	template<typename P>
	class iterator_t : public std::iterator<std::bidirectional_iterator_tag, P> {
	public:
		iterator_t() : list(0), entry(0) {}
		iterator_t& operator++() {
			inc();
			return *this;
		}
		iterator_t operator++(int) {
			iterator_t tmp = *this;
			inc();
			return tmp;
		}
		iterator_t& operator--() {
			dec();
			return *this;
		}
		iterator_t operator--(int) {
			iterator_t tmp = *this;
			dec();
			return tmp;
		}
		typename std::iterator<std::bidirectional_iterator_tag, P>::reference operator*() const {
			return entry->value;
		}
		typename std::iterator<std::bidirectional_iterator_tag, P>::pointer operator->() const {
			return &entry->value;
		}
		friend void swap(iterator_t& lhs, iterator_t& rhs) {
			std::swap(lhs.entry, rhs.entry);
		}
		friend bool operator==(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.entry == rhs.entry;
		}
		friend bool operator!=(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.entry != rhs.entry;
		}
	private:
		iterator_t(const List* list, entry_t* entry) : list(list), entry(entry) {}
		void inc() {
			entry = entry->next;
		}
		void dec() {
			if(entry) {
				entry = entry->prev;
			} else {
				entry = list->p_back;
			}
		}
		entry_t* entry;
		const List* list;
		friend class List;
	};
	
	typedef iterator_t<T> iterator;
	typedef iterator_t<const T> const_iterator;
	
	iterator begin() { return iterator(this, p_front); }
	const_iterator begin() const { return const_iterator(this, p_front); }
	const_iterator cbegin() const { return const_iterator(this, p_front); }
	
	iterator end() { return iterator(this, p_back ? p_back->next : 0); }
	const_iterator end() const { return const_iterator(this, p_back ? p_back->next : 0); }
	const_iterator cend() const { return const_iterator(this, p_back ? p_back->next : 0); }
	
	List& operator=(const List& other) {
		if(&other != this) {
			clear();
			append(other);
		}
		return *this;
	}
	
	void append(const List& other) {
		for(const T& obj : other) {
			push_back(obj);
		}
	}
	
	T& push_back() {
		return push_back(T());
	}
	
	T& push_back(const T& obj) {
		return *insert(end(), obj);
	}
	
	T& push_front() {
		return push_front(T());
	}
	
	T& push_front(const T& obj) {
		return *insert(begin(), obj);
	}
	
	bool pop_back(T& obj) {
		if(empty()) {
			return false;
		}
		obj = p_back->value;
		erase(iterator(this, p_back));
		return true;
	}
	
	T pop_back() {
		T obj;
		pop_back(obj);
		return obj;
	}
	
	bool pop_front(T& obj) {
		if(empty()) {
			return false;
		}
		obj = p_front->value;
		erase(begin());
		return true;
	}
	
	T pop_front() {
		T obj;
		pop_front(obj);
		return obj;
	}
	
	iterator insert(iterator iter, const T& obj) {
		count++;
		entry_t* next = iter.entry;
		entry_t* ptr;
		if(!p_front) {
			p_front = memory.create<entry_t>();
			p_back = p_front;
			ptr = p_front;
		} else if(next == p_back->next) {
			if(next) {
				ptr = next;
			} else {
				ptr = memory.create<entry_t>();
			}
			conn(p_back, ptr);
			p_back = p_back->next;
		} else {
			if(p_back->next) {
				ptr = p_back->next;
				conn(p_back, ptr->next);
			} else {
				ptr = memory.create<entry_t>();
			}
			conn(next->prev, ptr);
			conn(ptr, next);
			if(next == p_front) {
				p_front = ptr;
			}
		}
		ptr->value = obj;
		return iterator(this, ptr);
	}
	
	iterator erase(iterator iter) {
		count--;
		entry_t* old = iter.entry;
		if(p_front == p_back) {
			clear();
			return iterator(this, 0);
		} else if(old == p_back) {
			p_back = p_back->prev;
			return iterator(this, old);
		} else {
			entry_t* next = old->next;
			conn(old->prev, next);
			conn(old, p_back->next);
			conn(p_back, old);
			if(old == p_front) {
				p_front = next;
				p_front->prev = 0;
			}
			return iterator(this, next);
		}
	}
	
	void remove(const T& obj) {
		iterator iter = begin();
		while(iter != end()) {
			if(*iter == obj) {
				iter = erase(iter);
			} else {
				iter++;
			}
		}
	}
	
	T& operator[](int index) {
		auto iter = begin();
		for(int i = 0; i < index; ++i) {
			++iter;
		}
		return *iter;
	}
	
	T& front() {
		return p_front->value;
	}
	T& back() {
		return p_back->value;
	}
	
	const T& front() const {
		return p_front->value;
	}
	const T& back() const {
		return p_back->value;
	}
	
	int size() const {
		return count;
	}
	
	void clear() {
		entry_t* entry = p_front;
		while(entry) {
			entry->value.~T();
			entry = entry->next;
		}
		p_front = 0;
		p_back = 0;
		count = 0;
		memory.clear();
	}
	
	bool empty() const {
		return count == 0;
	}
	
private:
	void conn(entry_t* p0, entry_t* p1) {
		if(p0) {
			p0->next = p1;
		}
		if(p1) {
			p1->prev = p0;
		}
	}
	
private:
	BlockAllocator memory;
	entry_t* p_front;
	entry_t* p_back;
	int count;
	
};


}

#endif /* INCLUDE_PHY_LIST_H_ */
