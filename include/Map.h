/*
 * Map.h
 *
 *  Created on: May 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_MAP_H_
#define INCLUDE_PHY_MAP_H_

#include <assert.h>
#include <utility>

#include "Array.h"
#include "Queue.h"


namespace vnl {

/*
 * This is a hash map.
 * Maximum pair size is 4080 bytes.
 * Memory overhead is 3 pages + 16 bytes per element.
 */
template<typename K, typename V>
class Map {
public:
	Map(size_t rows = 512) {
		resize(rows);
	}
	
	Map(const Map& other) {
		*this = other;
	}
	
	~Map() {
		mem.clear();
		if(table) {
			table->free_all();
		}
	}
	
	Map& operator=(const Map& other) {
		resize(other.N);
		insert(other);
		return *this;
	}
	
	void insert(const Map& other) {
		for(auto& pair : other.entries()) {
			insert(pair.first, pair.second);
		}
	}
	
	Array<std::pair<K,V> > entries() const {
		Array<std::pair<K,V> > res;
		phy::Page* page = table;
		while(page) {
			for(int i = 0; i < M; ++i) {
				entry_t* row = page->get<entry_t*>(i);
				while(row) {
					res.push_back(row->pair);
					row = row->next;
				}
			}
			page = page->next;
		}
		return res;
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
				*p_row = (entry_t*)mem.alloc<entry_t>();
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
	
	size_t size() const {
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
	
	static const int M = phy::Page::size / sizeof(void*);
	
	void resize(size_t rows) {
		mem.clear();
		p_front = 0;
		N = rows;
		count = 0;
		index.clear();
		if(!table) {
			table = phy::Page::alloc();
		}
		int num_pages = N / M;
		int i = 0;
		phy::Page* page = table;
		while(true) {
			memset(page->mem, 0, phy::Page::size);
			index.push_back(page);
			i++;
			if(i >= num_pages) {
				break;
			}
			if(!page->next) {
				page->next = phy::Page::alloc();
			}
			page = page->next;
		}
	}
	
	void expand(int rows) {
		auto tmp = entries();
		assert(tmp.size() == count);
		resize(rows);
		int c = 0;
		for(auto& pair : tmp) {
			insert(pair.first, pair.second);
			c++;
		}
		assert(c == count);
		assert(tmp.size() == count);
	}
	
	bool find(const K& key, entry_t**& p_row, std::pair<K,V>*& val) {
		size_t ri = std::hash<K>{}(key) % N;
		size_t pi = ri / M;
		size_t qi = ri % M;
		phy::Page* page = index[pi];
		p_row = &page->get<entry_t*>(qi);
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
	
	phy::Region mem;
	
private:
	Array<phy::Page*> index;
	phy::Page* table = 0;
	entry_t* p_front = 0;
	size_t N = 0;
	size_t count = 0;
	
};


}

#endif /* INCLUDE_PHY_MAP_H_ */
