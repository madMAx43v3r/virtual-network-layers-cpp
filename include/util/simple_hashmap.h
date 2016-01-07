/*
 * simple_hashmap.h
 *
 *  Created on: Dec 22, 2015
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_SIMPLE_HASHMAP_H_
#define INCLUDE_UTIL_SIMPLE_HASHMAP_H_

#include "simple_queue.h"

namespace vnl { namespace util {

template<typename K, typename T>
class simple_hashmap {
public:
	simple_hashmap() : simple_hashmap(8) {}
	
	simple_hashmap(int capacity) : N(capacity), used(0) {
		map = new simple_queue<entry_t>[N];
	}
	
	simple_hashmap(const simple_hashmap&) = delete;
	simple_hashmap& operator=(const simple_hashmap&) = delete;
	
	~simple_hashmap() {
		delete [] map;
	}
	
	struct entry_t {
		K key;
		T val;
	};
	
	template<typename P>
	class iterator_t : public std::iterator<std::forward_iterator_tag, P> {
	public:
		iterator_t(simple_queue<entry_t>* queue_, simple_queue<entry_t>* end_) : queue(queue_), end(end_), skip(false) {
			if(queue != end) {
				iter = queue->begin();
				advance();
			}
		}
		iterator_t(const iterator_t&) = default;
		iterator_t& operator++() {
			if(!skip) {
				iter++;
				advance();
			}
			skip = false;
			return *this;
		}
		iterator_t operator++(int) {
			iterator_t prev = *this;
			if(!skip) {
				iter++;
				advance();
			}
			skip = false;
			return prev;
		}
		typename std::iterator<std::forward_iterator_tag, P>::reference operator*() const {
			return *iter;
		}
		typename std::iterator<std::forward_iterator_tag, P>::pointer operator->() const {
			return &(*iter);
		}
		friend void swap(iterator_t& lhs, iterator_t& rhs) {
			using std::swap;
			swap(lhs.queue, rhs.queue);
			swap(lhs.end, rhs.end);
			swap(lhs.iter, rhs.iter);
			swap(lhs.skip, rhs.skip);
		}
		friend bool operator==(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.iter == rhs.iter;
		}
		friend bool operator!=(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.iter != rhs.iter;
		}
	private:
		void advance() {
			while(iter == queue->end()) {
				queue++;
				if(queue != end) {
					iter = queue->begin();
				} else {
					reset();
					break;
				}
			}
		}
		void reset() {
			iter = typename simple_queue<entry_t>::iterator();
		}
		simple_queue<entry_t>* queue;
		simple_queue<entry_t>* end;
		typename simple_queue<entry_t>::iterator iter;
		bool skip;
		friend class simple_hashmap;
	};
	
	typedef iterator_t<entry_t> iterator;
	typedef iterator_t<const entry_t> const_iterator;
	
	iterator begin() { return iterator(map, map+N); }
	const_iterator begin() const { return const_iterator(map, map+N); }
	const_iterator cbegin() const { return const_iterator(map, map+N); }
	iterator end() { return iterator(map+N, map+N); }
	const_iterator end() const { return const_iterator(map+N, map+N); }
	const_iterator cend() const { return const_iterator(map+N, map+N); }
	
	T* get(const K& key) {
		simple_queue<entry_t>& queue = get_queue(key);
		entry_t* entry = get_entry(queue, key);
		if(entry) {
			return &entry->val;
		} else {
			return 0;
		}
	}
	
	T& put(const K& key) {
		return put(key, T());
	}
	
	T& put(const K& key, const T& val) {
		if(used > N) {
			expand();
		}
		simple_queue<entry_t>& queue = get_queue(key);
		entry_t* entry = get_entry(queue, key);
		if(!entry) {
			entry = &queue.push(entry_t());
			used++;
		}
		entry->key = key;
		entry->val = val;
		return entry->val;
	}
	
	void erase(const K& key) {
		simple_queue<entry_t>& queue = get_queue(key);
		for(auto iter = queue.begin(); iter != queue.end(); ++iter) {
			if(iter->key == key) {
				queue.erase(iter);
				used--;
				break;
			}
		}
	}
	
	void erase(iterator& iter) {
		iter.queue->erase(iter.iter);
		iter.advance();
		iter.skip = true;
		used--;
	}
	
	int size() {
		return used;
	}
	
protected:
	entry_t* get_entry(simple_queue<entry_t>& queue, const K& key) {
		for(entry_t& e : queue) {
			if(e.key == key) { return &e; }
		}
		return 0;
	}
	
	simple_queue<entry_t>& get_queue(const K& key) {
		size_t hash = std::hash<K>()(key);
		return map[hash & (N-1)];
	}
	
	void expand() {
		simple_hashmap newmap(N*2);
		for(int i = 0; i < N; ++i) {
			for(entry_t& e : map[i]) {
				newmap.put(e.key, e.val);
			}
		}
		std::swap(N, newmap.N);
		std::swap(used, newmap.used);
		std::swap(map, newmap.map);
	}
	
private:
	int N;
	int used;
	simple_queue<entry_t>* map;
	
};

}}

#endif /* INCLUDE_UTIL_SIMPLE_HASHMAP_H_ */
