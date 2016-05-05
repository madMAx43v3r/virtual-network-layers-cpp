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

#include "phy/Array.h"
#include "phy/Queue.h"


namespace vnl { namespace phy {

/*
 * This is a hash map.
 * Maximum pair size is 960 bytes at default.
 * B = block size
 */
template<typename K, typename V, int B = 4>
class Map {
public:
	Map() {
		assert(sizeof(K)+sizeof(V) <= Page::size/B - B*16);
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
	
	void insert(Map& other) {
		TPage<Row*>* page = other.table;
		int i = 0;
		while(page) {
			for(int k = 0; k < M && i < other.N; ++k) {
				Row* row = (*page)[k];
				for(std::pair<K,V>& pair : *row) {
					insert(pair.first, pair.second);
				}
				i++;
			}
			page = page->next;
		}
	}
	
	V& insert(const K& key, const V& val) {
		Row* row;
		V* ptr;
		if(find(key, row, ptr)) {
			*ptr = val;
		} else {
			if(count >= N*B) {
				expand(N*2);
				return insert(key, val);
			}
			ptr = &row->push(val);
			count++;
		}
		return *ptr;
	}
	
	V& operator[](const K& key) {
		Row* row;
		V* ptr;
		if(find(key, row, ptr)) {
			return *ptr;
		} else {
			return insert(key, V());
		}
	}
	
	V* find(const K& key) {
		Row* row;
		V* ptr;
		if(find(key, row, ptr)) {
			return ptr;
		}
		return 0;
	}
	
	void erase(const K& key) {
		Row* row;
		V* ptr;
		if(find(key, row, ptr)) {
			auto end_ = row->end();
			for(auto iter = row->begin(); iter != end_; ++iter) {
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
			table = TPage<Row*>::alloc();
		}
		N = rows;
		index.clear();
		index.push_back(table);
		int pos = 0;
		for(int i = 0; i < N; ++i) {
			if(pos >= M) {
				if(!table->next) {
					table->next = TPage<Row*>::alloc();
				}
				table = table->next;
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
	
	Region mem;
	
protected:
	typedef Queue<std::pair<K,V>,B> Row;
	
	static const int M = TPage<Row*>::M;
	
	Map(int rows) {
		clear(rows);
	}
	
	void expand(int rows) {
		Map tmp(rows);
		tmp.insert(*this);
		*this = tmp;
	}
	
	bool find(const K& key, Row*& row, V*& val) {
		size_t ri = std::hash(key) % N;
		size_t pi = ri / M;
		size_t qi = ri % M;
		TPage<Row*>* page = index[pi];
		row = (*page)[qi];
		for(std::pair<K,V>& pair : *row) {
			if(pair.first == key) {
				val = &pair.second;
				return true;
			}
		}
		return false;
	}
	
private:
	Array<TPage<Row*>*> index;
	TPage<Row*>* table = 0;
	size_t N = 0;
	size_t count = 0;
	
};


}}

#endif /* INCLUDE_PHY_MAP_H_ */
