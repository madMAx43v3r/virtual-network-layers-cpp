/*
 * linear_hashmap.h
 *
 *  Created on: Dec 20, 2015
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_LINEAR_HASHMAP_H_
#define INCLUDE_UTIL_LINEAR_HASHMAP_H_

#include <vector>
#include <utility>

namespace vnl { namespace util {

template<typename K, typename T>
class linear_hashmap {
public:
	linear_hashmap(int factor = 4) : factor(factor) {
		array.resize(N);
	}
	
	T* get(const K& key) {
		entry_t& entry = get_entry(key);
		if(!entry.isfree && entry.key == key) {
			return &entry.val;
		} else {
			return 0;
		}
	}
	
	bool put(const K& key, const T& val) {
		entry_t& entry = get_entry(key);
		if(entry.isfree) {
			entry.key = key;
			entry.val = val;
			entry.isfree = false;
			used++;
			if(N/used < factor) {
				expand();
			}
			return true;
		} else {
			return false;
		}
	}
	
	void erase(const K& key) {
		entry_t& entry = get_entry(key);
		if(!entry.isfree && entry.key == key) {
			entry.isfree = true;
			used--;
		}
	}
	
	size_t size() {
		return used;
	}
	
protected:
	struct entry_t {
		K key;
		T val;
		bool isfree = true;
	};
	
	entry_t& get_entry(const K& key) {
		return get_entry(array, N, key);
	}
	
	entry_t& get_entry(std::vector<entry_t>& vec, size_t mod, const K& key) {
		size_t hash = std::hash<K>()(key);
		return vec[hash % mod];
	}
	
	void expand() {
		size_t newN = N*2;
		std::vector<entry_t> newarray(newN);
		for(const entry_t& e : array) {
			if(!e.isfree) {
				get_entry(newarray, newN, e.key) = e;
			}
		}
		array = newarray;
		N = newN;
	}
	
private:
	int factor;
	size_t N = 8;
	size_t used = 0;
	std::vector<entry_t> array;
	
};

}}

#endif /* INCLUDE_UTIL_LINEAR_HASHMAP_H_ */
