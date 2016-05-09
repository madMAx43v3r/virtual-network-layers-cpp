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
 * Memory overhead is 50 bytes per element.
 */
template<typename K, typename V>
class Map {
public:
	Map(size_t rows = 64) {
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
		int i = 0;
		while(page && i < N) {
			int max = N - i;
			if(max > M) { max = M; }
			for(int k = 0; k < max; ++k) {
				Row* row = page->get<Row*>(k);
				for(auto& pair : *row) {
					res.push_back(pair);
				}
			}
			page = page->next;
			i += max;
		}
		return res;
	}
	
	V& insert(const K& key, const V& val) {
		Row* row;
		std::pair<K,V>* ptr;
		if(find(key, row, ptr)) {
			ptr->second = val;
		} else {
			if(count >= N) {
				expand(N*2);
				return insert(key, val);
			}
			ptr = &row->push(std::make_pair(key, val));
			count++;
		}
		return ptr->second;
	}
	
	V& operator[](const K& key) {
		Row* row;
		std::pair<K,V>* ptr;
		if(find(key, row, ptr)) {
			return ptr->second;
		} else {
			return insert(key, V());
		}
	}
	
	V* find(const K& key) {
		Row* row;
		std::pair<K,V>* ptr;
		if(find(key, row, ptr)) {
			return &ptr->second;
		}
		return 0;
	}
	
	void erase(const K& key) {
		Row* row;
		std::pair<K,V>* ptr;
		if(find(key, row, ptr)) {
			int n = row->size();
			for(int i = 0; i < n; ++i) {
				const auto& pair = row->front();
				if(pair.first != key) {
					row->push(pair);
				} else {
					count--;
				}
				row->pop();
			}
		}
	}
	
	void clear() {
		resize(N);
	}
	
	void resize(size_t rows) {
		mem.clear();
		if(rows < 1) {
			rows = 1;
		}
		if(!table) {
			table = phy::Page::alloc();
		}
		N = rows;
		count = 0;
		index.clear();
		index.push_back(table);
		int pos = 0;
		phy::Page* page = table;
		for(int i = 0; i < N; ++i) {
			if(pos >= M) {
				if(!page->next) {
					page->next = phy::Page::alloc();
				}
				page = page->next;
				index.push_back(page);
				pos = 0;
			}
			page->get<Row*>(pos) = new(mem.alloc<Row>()) Row(mem);
			pos++;
		}
	}
	
	size_t size() const {
		return count;
	}
	
	bool empty() const {
		return count == 0;
	}
	
protected:
	typedef Queue<std::pair<K,V>,1> Row;
	
	static const int M = phy::Page::size / sizeof(void*);
	
	void expand(int rows) {
		auto tmp = entries();
		resize(rows);
		for(auto& pair : tmp) {
			insert(pair.first, pair.second);
		}
	}
	
	bool find(const K& key, Row*& row, std::pair<K,V>*& val) {
		size_t ri = std::hash<K>{}(key) % N;
		size_t pi = ri / M;
		size_t qi = ri % M;
		phy::Page* page = index[pi];
		row = page->get<Row*>(qi);
		for(std::pair<K,V>& pair : *row) {
			if(pair.first == key) {
				val = &pair;
				return true;
			}
		}
		return false;
	}
	
	phy::Region mem;
	
private:
	Array<phy::Page*> index;
	phy::Page* table = 0;
	size_t N = 0;
	size_t count = 0;
	
};


}

#endif /* INCLUDE_PHY_MAP_H_ */
