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
template<typename T, typename TPage = Memory<VNL_PAGE_SIZE> >
class Tree {
public:
	Tree() : p_root(0), depth(0), count(0) {
		assert(M > 0);
	}
	
	Tree(const Tree& other) : p_root(0), depth(0), count(0) {
		*this = other;
	}
	
	~Tree() {
		clear();
	}
	
public:
	template<typename P>
	class iterator_t : public std::iterator<std::forward_iterator_tag, P> {
	public:
		iterator_t() : tree(0), index(0) {}
		iterator_t(const iterator_t& other) : tree(other.tree), index(other.index) {}
		iterator_t& operator++() {
			inc();
			return *this;
		}
		iterator_t operator++(int) {
			iterator_t tmp = *this;
			inc();
			return tmp;
		}
		typename std::iterator<std::forward_iterator_tag, P>::reference operator*() const {
			return get();
		}
		typename std::iterator<std::forward_iterator_tag, P>::pointer operator->() const {
			return &get();
		}
		friend void swap(iterator_t& lhs, iterator_t& rhs) {
			std::swap(lhs.tree, rhs.tree);
			std::swap(lhs.index, rhs.index);
		}
		friend bool operator==(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.index == rhs.index;
		}
		friend bool operator!=(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.index != rhs.index;
		}
	private:
		iterator_t(Tree* tree, int index) : tree(tree), index(index) {}
		void inc() {
			index++;
		}
		P& get() const {
			return (*tree)[index];
		}
		Tree* tree;
		int index;
		friend class Tree;
	};
	
	typedef iterator_t<T> iterator;
	typedef iterator_t<const T> const_iterator;
	
	iterator begin() { return iterator(this, 0); }
	const_iterator begin() const { return const_iterator((Tree*)this, 0); }
	const_iterator cbegin() const { return const_iterator((Tree*)this, 0); }
	
	iterator end() { return iterator(this, count); }
	const_iterator end() const { return const_iterator((Tree*)this, count); }
	const_iterator cend() const { return const_iterator((Tree*)this, count); }
	
	Tree& operator=(const Tree& other) {
		resize(other.size());
		for(int i = 0; i < count; ++i) {
			(*this)[i] = other[i];
		}
		return *this;
	}
	
	T& operator[](int index) {
		return *((T*)walk(p_root, 0, index));
	}
	
	const T& operator[](int index) const {
		return *((const T*)walk(p_root, 0, index));
	}
	
	void resize(int n) {
		clear();
		count = n;
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
	
	int size() const {
		return count;
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
	int count;
	
};


} // vnl

#endif /* INCLUDE_VNL_TREE_H_ */
