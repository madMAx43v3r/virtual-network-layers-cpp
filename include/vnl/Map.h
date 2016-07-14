/*
 * Map.h
 *
 *  Created on: May 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_MAP_H_
#define INCLUDE_PHY_MAP_H_

#include <utility>
#include <string.h>

#include <vnl/Array.h>


namespace vnl {

/*
 * This is a hash map.
 * Maximum pair size is 4080 bytes.
 * Memory overhead is minimum 2 pages + 16 bytes per element.
 * Not recommended for more than 10000 elements.
 */
template<typename K, typename V>
class Map {
public:
	Map() {
		Map::resize((int)Page::size/sizeof(void*));
	}
	
	Map(const Map& other) {
		*this = other;
	}
	
	~Map() {
		destroy();
	}
	
	Map& operator=(const Map& other) {
		resize(other.N);
		insert(other);
		return *this;
	}
	
	Array<std::pair<K,V> > entries() const {
		Array<std::pair<K,V> > res;
		for(entry_t* row : table) {
			while(row) {
				res.push_back(row->pair);
				row = row->next;
			}
		}
		return res;
	}
	
	Array<K> keys() const {
		Array<K> res;
		for(entry_t* row : table) {
			while(row) {
				res.push_back(row->pair.first);
				row = row->next;
			}
		}
		return res;
	}
	
	Array<V> values() const {
		Array<V> res;
		for(entry_t* row : table) {
			while(row) {
				res.push_back(row->pair.second);
				row = row->next;
			}
		}
		return res;
	}
	
	void insert(const Map& other) {
		for(auto& pair : other.entries()) {
			insert(pair.first, pair.second);
		}
	}
	
	V& insert(const K& key, const V& val) {
		entry_t** p_row;
		std::pair<K,V>* ptr;
		if(find(key, p_row, ptr)) {
			ptr->second = val;
		} else {
			if(count >= N) {
				expand(N*2);
				return insert(key, val);
			}
			if(p_front) {
				*p_row = p_front;
				p_front = p_front->next;
			} else {
				*p_row = mem.create<entry_t>();
			}
			entry_t* row = *p_row;
			row->pair = std::make_pair(key, val);
			row->next = 0;
			ptr = &row->pair;
			count++;
		}
		return ptr->second;
	}
	
	V& operator[](const K& key) {
		entry_t** p_row;
		std::pair<K,V>* ptr;
		if(find(key, p_row, ptr)) {
			return ptr->second;
		} else {
			return insert(key, V());
		}
	}
	
	V* find(const K& key) {
		entry_t** p_row;
		std::pair<K,V>* ptr;
		if(find(key, p_row, ptr)) {
			return &ptr->second;
		}
		return 0;
	}
	
	void erase(const K& key) {
		entry_t** p_row;
		std::pair<K,V>* ptr;
		if(find(key, p_row, ptr)) {
			entry_t* row = *p_row;
			*p_row = row->next;
			row->next = p_front;
			p_front = row;
			count--;
		}
	}
	
	void clear() {
		resize(N);
	}
	
	int size() const {
		return count;
	}
	
	bool empty() const {
		return count == 0;
	}
	
protected:
	struct entry_t {
		std::pair<K,V> pair;
		entry_t* next;
	};
	
	void destroy() {
		for(entry_t* row : table) {
			while(row) {
				entry_t* next = row->next;
				row->~entry_t();
				row = row->next;
			}
		}
		entry_t* entry = p_front;
		while(entry) {
			entry_t* next = entry->next;
			entry->~entry_t();
			entry = next;
		}
		mem.clear();
	}
	
	void resize(int rows) {
		destroy();
		N = rows;
		count = 0;
		p_front = 0;
		table.clear();
		for(int i = 0; i < N; ++i) {
			table.push_back(0);
		}
	}
	
	void expand(int rows) {
		auto tmp = entries();
		resize(rows);
		for(auto& pair : tmp) {
			insert(pair.first, pair.second);
		}
	}
	
	bool find(const K& key, entry_t**& p_row, std::pair<K,V>*& val) {
		int ind = std::hash<K>{}(key) % N;
		p_row = &table[ind];
		while(true) {
			entry_t* row = *p_row;
			if(!row) {
				break;
			}
			if(row->pair.first == key) {
				val = &row->pair;
				return true;
			}
			p_row = &row->next;
		}
		return false;
	}
	
	PageAllocator mem;
	
private:
	Array<entry_t*> table;
	entry_t* p_front = 0;
	int N = 0;
	int count = 0;
	
};


}

#endif /* INCLUDE_PHY_MAP_H_ */
