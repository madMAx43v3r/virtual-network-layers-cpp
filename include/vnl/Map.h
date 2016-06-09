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

#include "Array.h"


namespace vnl {

/*
 * This is a hash map.
 * Maximum pair size is 4080 bytes.
 * Memory overhead is 3 pages + 16 bytes per element.
 */
template<typename K, typename V>
class Map {
public:
	Map() {
		resize(512);
	}
	
	Map(const Map& other) {
		*this = other;
	}
	
	~Map() {
		destroy();
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
		Page* page = table;
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
				*p_row = (entry_t*)mem.create<entry_t>();
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
	
	static const int M = Page::size / sizeof(void*);
	
	void destroy() {
		Page* page = table;
		while(page) {
			for(int i = 0; i < M; ++i) {
				entry_t* row = page->get<entry_t*>(i);
				while(row) {
					entry_t* next = row->next;
					row->~entry_t();
					row = row->next;
				}
			}
			page = page->next;
		}
		entry_t* entry = p_front;
		while(entry) {
			entry_t* next = entry->next;
			entry->~entry_t();
			entry = next;
		}
		mem.clear();
	}
	
	void resize(size_t rows) {
		destroy();
		p_front = 0;
		N = rows;
		count = 0;
		index.clear();
		if(!table) {
			table = Page::alloc();
		}
		int num_pages = N / M;
		int i = 0;
		Page* page = table;
		while(true) {
			memset(page->mem, 0, Page::size);
			index.push_back(page);
			i++;
			if(i >= num_pages) {
				break;
			}
			if(!page->next) {
				page->next = Page::alloc();
			}
			page = page->next;
		}
	}
	
	void expand(int rows) {
		auto tmp = entries();
		resize(rows);
		int c = 0;
		for(auto& pair : tmp) {
			insert(pair.first, pair.second);
			c++;
		}
	}
	
	bool find(const K& key, entry_t**& p_row, std::pair<K,V>*& val) {
		size_t ri = std::hash<K>{}(key) % N;
		size_t pi = ri / M;
		size_t qi = ri % M;
		Page* page = index[pi];
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
	
	Region mem;
	
private:
	Array<Page*> index;
	Page* table = 0;
	entry_t* p_front = 0;
	size_t N = 0;
	size_t count = 0;
	
};


}

#endif /* INCLUDE_PHY_MAP_H_ */
