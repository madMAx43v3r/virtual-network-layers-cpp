/*
 * Map.h
 *
 *  Created on: May 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_MAP_H_
#define INCLUDE_PHY_MAP_H_

#include <vnl/Util.h>
#include <vnl/Tree.h>
#include <vnl/List.h>

#include <utility>


namespace vnl {

/*
 * This is a hash map with O(log(n)) complexity.
 * Maximum pair size is VNL_PAGE_SIZE-16 bytes.
 * Memory overhead is minimum 1 page + 1 block + 16 bytes per element.
 */
template<typename K, typename V, typename TPage = Memory<VNL_PAGE_SIZE> >
class Map {
public:
	Map() {
		resize(32);
	}
	
	Map(const Map& other) {
		*this = other;
	}
	
protected:
	typedef List<vnl::pair<K,V>, TPage> list_t;
	typedef typename list_t::entry_t entry_t;
	typedef Tree<entry_t*> tree_t;
	
public:
	typedef typename list_t::iterator iterator;
	typedef typename list_t::const_iterator const_iterator;
	
	iterator begin() { return list.begin(); }
	const_iterator begin() const { return list.begin(); }
	const_iterator cbegin() const { return list.cbegin(); }
	
	iterator end() { return list.end(); }
	const_iterator end() const { return list.end(); }
	const_iterator cend() const { return list.cend(); }
	
	Map& operator=(const Map& other) {
		resize(other.N);
		insert(other);
		return *this;
	}
	
	/*
	 * Retruns a copy of all entries.
	 */
	Array<vnl::pair<K,V> > entries() const {
		Array<vnl::pair<K,V> > res;
		for(const_iterator it = begin(); it != end(); ++it) {
			res.push_back(*it);
		}
		return res;
	}
	
	/*
	 * Retruns a copy of all keys.
	 */
	Array<K> keys() const {
		Array<K> res;
		for(const_iterator it = begin(); it != end(); ++it) {
			res.push_back(it->first);
		}
		return res;
	}
	
	/*
	 * Retruns a copy of all values.
	 */
	Array<V> values() const {
		Array<V> res;
		for(const_iterator it = begin(); it != end(); ++it) {
			res.push_back(it->second);
		}
		return res;
	}
	
	void insert(const Map& other) {
		for(const_iterator it = other.begin(); it != other.end(); ++it) {
			insert(it->first, it->second);
		}
	}
	
	V& insert(const K& key, const V& val) {
		entry_t** p_row;
		iterator it = find(key, p_row);
		if(it != list.end()) {
			return it->second = val;
		} else {
			if(*p_row) {
				resize(N*2);
				return insert(key, val);
			}
			it = list.push_back(vnl::make_pair(key, val));
			*p_row = it.get_entry();
			return it->second;
		}
	}
	
	V& operator[](const K& key) {
		entry_t** p_row;
		iterator it = find(key, p_row);
		if(it != list.end()) {
			return it->second;
		} else {
			return insert(key, V());
		}
	}
	
	V* find(const K& key) {
		entry_t** p_row;
		iterator it = find(key, p_row);
		if(it != list.end()) {
			return &it->second;
		}
		return 0;
	}
	
	void erase(const K& key) {
		entry_t** p_row;
		iterator it = find(key, p_row);
		if(it != list.end()) {
			list.erase(it);
			*p_row = 0;
		}
	}
	
	void clear() {
		list.clear();
		resize(32);
	}
	
	int size() const {
		return list.size();
	}
	
	bool empty() const {
		return size() == 0;
	}
	
protected:
	int get_index(const K& key) {
		return std::hash<K>{}(key) % N;
	}
	
	void resize(int rows) {
		assert(rows > 0);
		N = rows;
		tree.resize(N);
		for(iterator it = list.begin(); it != list.end(); ++it) {
			tree[get_index(it->first)] = it.get_entry();
		}
	}
	
	iterator find(const K& key, entry_t**& p_row) {
		int index = get_index(key);
		entry_t*& row = tree[index];
		p_row = &row;
		if(row) {
			if(row->value.first == key) {
				return iterator(&list, row);
			}
		}
		return list.end();
	}
	
private:
	int N;
	tree_t tree;
	list_t list;
	
};


} // vnl

#endif /* INCLUDE_PHY_MAP_H_ */
