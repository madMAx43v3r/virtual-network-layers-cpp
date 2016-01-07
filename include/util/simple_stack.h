/*
 * simple_stack.h
 *
 *  Created on: Dec 23, 2015
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_SIMPLE_STACK_H_
#define INCLUDE_UTIL_SIMPLE_STACK_H_

#include <vector>
#include <stack>
//#include <iterator>

namespace vnl { namespace util {

template<typename T>
using simple_stack = std::stack<T, std::vector<T> >;

/*template<typename T>
class simple_stack {
public:
	simple_stack() : avail(0), front(0), free(0) {}
	
	simple_stack(const simple_stack&) = delete;
	
	~simple_stack() {
		destroy(front);
		destroy(free);
	}
	
	struct node_t {
		node_t* next;
		T obj;
	};
	
	T& push(const T& obj) {
		node_t* node;
		if(free) {
			node = free;
			free = free->next;
		} else {
			node = new node_t();
		}
		node->next = front;
		node->obj = obj;
		front = node;
		avail++;
		return node->obj;
	}
	
	bool pop(T& obj) {
		if(front) {
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
	
	int size() {
		return avail;
	}
	
protected:
	void push_node(node_t* node) {
		node->next = free;
		free = node;
	}
	
	void destroy(node_t* node) {
		while(node) {
			node_t* next = node->next;
			delete node;
			node = next;
		}
	}
	
private:
	int avail;
	node_t* front;
	node_t* free;
	
	
};*/

}}

#endif /* INCLUDE_UTIL_SIMPLE_STACK_H_ */
