/*
 * Tree.h
 *
 *  Created on: Nov 2, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_TREE_H_
#define INCLUDE_VNL_TREE_H_

#include <vnl/Memory.h>


namespace vnl {

/*
 * This is a tree of pointers with O(log(n)) complexity.
 */
template<typename T, typename TPage = Memory<VNL_BLOCK_SIZE> >
class Tree {
public:
	Tree() : p_root(0), depth(0) {
		assert(M > 0);
	}
	
	Tree(const Tree& other) : p_root(0), depth(0) {
		assert(false); // not implemented
	}
	
	~Tree() {
		clear();
	}
	
	Tree& operator=(const Tree& other) {
		assert(false); // not implemented
		return *this;
	}
	
	T& operator[](int index) {
		assert(p_root);
		return *((T*)walk(p_root, 0, index));
	}
	
	const T& operator[](int index) const {
		assert(p_root);
		return *((const T*)walk(p_root, 0, index));
	}
	
	void resize(int n) {
		clear();
		p_root = alloc();
		n /= M;
		while(n > 0) {
			depth++;
			n /= N;
		}
	}
	
	void clear() {
		if(p_root) {
			clear(p_root, 0);
			p_root = 0;
			depth = 0;
		}
	}
	
protected:
	TPage* alloc() const {
		TPage* block = TPage::alloc();
		memset(block->mem, 0, TPage::size);
		return block;
	}
	
	void* walk(TPage* block, int level, int index) const {
		if(level == depth) {
			return &block->template type_at_index<T>(index % M);
		}
		TPage*& next = block->template type_at_index<TPage*>(index % N);
		if(!next) {
			next = alloc();
		}
		return walk(next, level+1, index/N);
	}
	
	void clear(TPage* block, int level) {
		if(level < depth) {
			for(int i = 0; i < N; ++i) {
				TPage* ptr = block->template type_at_index<TPage*>(i);
				if(ptr) {
					clear(ptr, level+1);
				}
			}
		}
		block->free();
	}
	
protected:
	static const int N = TPage::size / sizeof(void*);
	static const int M = TPage::size / sizeof(T);
	
	TPage* p_root;
	int depth;
	
};


} // vnl

#endif /* INCLUDE_VNL_TREE_H_ */
