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
 * Maximum pair size is 960 bytes at default.
 * B = block size
 */
template<typename K, typename V, int B = 4>
class Map {
public:
	Map() {
		assert(sizeof(K)+sizeof(V) <= phy::Page::size/B - B*16);
		clear();
	}
	
	Map(const Map& other) {
		clear(other.N);
		insert(other);
	}
	
	~Map() {
		mem.clear();
		if(table) {
			table->free_all();
		}
	}
	
	Map& operator=(const Map& other) {
		clear(other.N);
		insert(other);
		return *this;
	}
	
	void insert(const Map& other) {
		phy::TPage<Row*>* page = other.table;
		int i = 0;
		while(page) {
			for(int k = 0; k < M && i < other.N; ++k) {
				Row* row = (*page)[k];
				for(std::pair<K,V>& pair : *row) {
					insert(pair.first, pair.second);
				}
				i++;
			}
			page = (phy::TPage<Row*>*)page->next;
		}
	}
	
	V& insert(const K& key, const V& val) {
		Row* row;
		std::pair<K,V>* ptr;
		if(find(key, row, ptr)) {
			ptr->second = val;
		} else {
			if(count >= N*B) {
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
			auto row_end = row->end();
			while(true) {
				auto iter = row->begin();
				if(iter == row_end) {
					break;
				}
				if(iter->first != key) {
					row->push(*iter);
				} else {
					count--;
				}
				row->pop();
			}
		}
	}
	
	void clear(size_t rows = 64) {
		mem.clear();
		if(rows < 1) {
			rows = 1;
		}
		if(!table) {
			table = phy::TPage<Row*>::alloc();
		}
		N = rows;
		index.clear();
		index.push_back(table);
		int pos = 0;
		for(int i = 0; i < N; ++i) {
			if(pos >= M) {
				if(!table->next) {
					table->next = phy::TPage<Row*>::alloc();
				}
				table = (phy::TPage<Row*>*)table->next;
				index.push_back(table);
				pos = 0;
			}
			(*table)[pos] = new(mem.alloc<Row>()) Row(&mem);
			pos++;
		}
		count = 0;
	}
	
	size_t size() const {
		return count;
	}
	
	bool empty() const {
		return count == 0;
	}
	
protected:
	typedef Queue<std::pair<K,V>,B> Row;
	
	static const int M = phy::TPage<Row*>::M;
	
	Map(int rows) {
		clear(rows);
	}
	
	void expand(int rows) {
		Map tmp(rows);
		tmp.insert(*this);
		*this = tmp;
	}
	
	bool find(const K& key, Row*& row, std::pair<K,V>*& val) {
		size_t ri = std::hash<K>{}(key) % N;
		size_t pi = ri / M;
		size_t qi = ri % M;
		phy::TPage<Row*>* page = index[pi];
		row = (*page)[qi];
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
	Array<phy::TPage<Row*>*> index;
	phy::TPage<Row*>* table = 0;
	size_t N = 0;
	size_t count = 0;
	
};


}

#endif /* INCLUDE_PHY_MAP_H_ */
