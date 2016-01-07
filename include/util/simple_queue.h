/*
 * simple_queue.h
 *
 *  Created on: Dec 21, 2015
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_SIMPLE_QUEUE_H_
#define INCLUDE_UTIL_SIMPLE_QUEUE_H_

#include <iterator>

namespace vnl { namespace util {

template<typename T>
class simple_queue {
public:
	simple_queue() : avail(0) {
		front = new node_t();
		front->next = 0;
		write = front;
	}
	
	simple_queue(const simple_queue&) = delete;
	
	simple_queue& operator=(const simple_queue& other) {
		for(const T& obj : other) {
			push(obj);
		}
		return *this;
	}
	
	~simple_queue() {
		node_t* node = front;
		while(node) {
			node_t* next = node->next;
			delete node;
			node = next;
		}
	}
	
	struct node_t {
		node_t* next;
		T obj;
	};
	
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
			return current->obj;
		}
		typename std::iterator<std::forward_iterator_tag, P>::pointer operator->() const {
			return &current->obj;
		}
		friend void swap(iterator_t& lhs, iterator_t& rhs) {
			using std::swap;
			swap(lhs.current, rhs.current);
			swap(lhs.prev, rhs.prev);
			swap(lhs.skip, rhs.skip);
		}
		friend bool operator==(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.current == rhs.current;
		}
		friend bool operator!=(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.current != rhs.current;
		}
	private:
		iterator_t(node_t* node) : iterator_t(node, 0) {}
		iterator_t(node_t* node, node_t* prev) : current(node), prev(prev), skip(false) {}
		void advance() {
			if(!skip) {
				prev = current;
				current = current->next;
			}
			skip = false;
		}
		node_t* ptr() const {
			return current;
		}
		node_t* ptr_prev() const {
			return prev;
		}
		node_t* current;
		node_t* prev;
		bool skip;
		friend class simple_queue;
	};
	
	typedef iterator_t<T> iterator;
	typedef iterator_t<const T> const_iterator;
	
	iterator begin() { return iterator(front); }
	const_iterator begin() const { return const_iterator(front); }
	const_iterator cbegin() const { return const_iterator(front); }
	iterator end() { return iterator(write); }
	const_iterator end() const { return const_iterator(write); }
	const_iterator cend() const { return const_iterator(write); }
	
	T& push(const T& obj) {
		node_t* node = write;
		if(write->next == 0) {
			push_node(new node_t());
		}
		write = write->next;
		node->obj = obj;
		avail++;
		return node->obj;
	}
	
	bool pop(T& obj) {
		if(avail > 0) {
			obj = front->obj;
			node_t* next = front->next;
			push_node(front);
			front = next;
			avail--;
			return true;
		} else {
			return false;
		}
	}
	
	T& insert(iterator& iter, const T& obj) {
		if(iter.ptr() == write) {
			return push(obj);
		}
		node_t* node;
		if(write->next == 0) {
			node = new node_t();
		} else {
			node = write->next;
			write->next = node->next;
		}
		if(iter.ptr_prev()) {
			iter.ptr_prev()->next = node;
		} else {
			front = node;
		}
		node->next = iter.ptr();
		node->obj = obj;
		avail++;
		return node->obj;
	}
	
	void erase(iterator& iter) {
		node_t* next = iter.ptr()->next;
		if(iter.ptr_prev()) {
			iter.ptr_prev()->next = next;
		} else {
			front = next;
		}
		push_node(iter.ptr());
		iter.current = next;
		iter.skip = true;
		avail--;
	}
	
	void erase(const T& obj) {
		for(auto iter = begin(); iter != end(); ++iter) {
			if(*iter == obj) {
				erase(iter);
				return;
			}
		}
	}
	
	void clear() {
		write = front;
		avail = 0;
	}
	
	int size() {
		return avail;
	}
	
protected:
	void push_node(node_t* node) {
		node->next = write->next;
		write->next = node;
	}
	
private:
	int avail;
	node_t* front;
	node_t* write;
	
};

}}

#endif /* INCLUDE_UTIL_SIMPLE_QUEUE_H_ */
