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

#include <utility>


namespace vnl {

/*
 * This is a hash map.
 * Maximum pair size is VNL_PAGE_SIZE-8 bytes.
 * Memory overhead is minimum 2 pages + 16 bytes per element.
 */
template<typename K, typename V, typename TPage = Memory<VNL_PAGE_SIZE> >
class Map {
public:
	Map() : p_front(0), N(0), count(0) {
		resize((int)TPage::size/sizeof(void*));
	}
	
	Map(const Map& other) : p_front(0), N(0), count(0) {
		*this = other;
	}
	
	~Map() {
		destroy();
	}
	
protected:
	struct entry_t {
		vnl::pair<K,V> pair;
		entry_t* next;
		entry_t() : next(0) {}
	};
	
	typedef Tree<entry_t*> table_t;
	
public:
	template<typename P>
	class iterator_t : public std::iterator<std::forward_iterator_tag, P> {
	public:
		iterator_t() : table(0), index(0), end(0), entry(0) {}
		iterator_t(const iterator_t& other) : table(other.table), index(other.index), end(other.end), entry(other.entry) {}
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
			return entry->pair;
		}
		typename std::iterator<std::forward_iterator_tag, P>::pointer operator->() const {
			return &entry->pair;
		}
		friend void swap(iterator_t& lhs, iterator_t& rhs) {
			std::swap(lhs.table, rhs.table);
			std::swap(lhs.index, rhs.index);
			std::swap(lhs.end, rhs.end);
			std::swap(lhs.entry, rhs.entry);
		}
		friend bool operator==(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.index == rhs.index && lhs.entry == rhs.entry;
		}
		friend bool operator!=(const iterator_t& lhs, const iterator_t& rhs) {
			return lhs.index != rhs.index || lhs.entry != rhs.entry;
		}
	private:
		iterator_t(table_t* table, int index, int end, entry_t* ptr = 0)
			:	table(table), index(index), end(end), entry(ptr)
		{
			if(index < end) {
				if(!entry) {
					entry = (*table)[index];
				}
				search();
			}
		}
		void inc() {
			if(entry) {
				entry = entry->next;
			}
			search();
		}
		void search() {
			while(!entry) {
				index++;
				if(index >= end) {
					break;
				}
				entry = (*table)[index];
			}
		}
		int index;
		int end;
		table_t* table;
		entry_t* entry;
		friend class Map;
	};
	
	typedef iterator_t<vnl::pair<K,V> > iterator;
	typedef iterator_t<const vnl::pair<K,V> > const_iterator;
	
	iterator begin() { return iterator(&table, 0, N); }
	const_iterator begin() const { return const_iterator((table_t*)&table, 0, N); }
	const_iterator cbegin() const { return const_iterator((table_t*)&table, 0, N); }
	
	iterator end() { return iterator(&table, N, N, 0); }
	const_iterator end() const { return const_iterator((table_t*)&table, N, N); }
	const_iterator cend() const { return const_iterator((table_t*)&table, N, N); }
	
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
		vnl::pair<K,V>* ptr;
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
				*p_row = memory.template create<entry_t>();
			}
			entry_t* row = *p_row;
			row->pair = vnl::make_pair(key, val);
			row->next = 0;
			ptr = &row->pair;
			count++;
		}
		return ptr->second;
	}
	
	V& operator[](const K& key) {
		entry_t** p_row;
		vnl::pair<K,V>* ptr;
		if(find(key, p_row, ptr)) {
			return ptr->second;
		} else {
			return insert(key, V());
		}
	}
	
	V* find(const K& key) {
		entry_t** p_row;
		vnl::pair<K,V>* ptr;
		if(find(key, p_row, ptr)) {
			return &ptr->second;
		}
		return 0;
	}
	
	void erase(const K& key) {
		entry_t** p_row;
		vnl::pair<K,V>* ptr;
		if(find(key, p_row, ptr)) {
			remove(p_row);
		}
	}
	
	iterator erase(iterator pos) {
		assert(pos.entry);
		entry_t** p_row = (entry_t**)&(*pos.index);
		while(true) {
			if(*p_row == pos.entry) {
				remove(p_row);
				break;
			}
			p_row = &(*p_row)->next;
		}
		return iterator(&table, pos.index, pos.end, *p_row);
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
	void remove(entry_t** p_row) {
		entry_t* row = *p_row;
		*p_row = row->next;
		row->next = p_front;
		p_front = row;
		count--;
	}
	
	void destroy() {
		for(int i = 0; i < N; ++i) {
			entry_t* row = table[i];
			if(row) {
				destroy_list(row);
			}
		}
		destroy_list(p_front);
		memory.clear();
	}
	
	void destroy_list(entry_t* row) {
		while(row) {
			entry_t* next = row->next;
			row->~entry_t();
			row = row->next;
		}
	}
	
	void resize(int rows) {
		destroy();
		N = rows;
		count = 0;
		p_front = 0;
		table.resize(N);
	}
	
	void expand(int rows) {
		Array<vnl::pair<K,V> > tmp = entries();
		resize(rows);
		for(vnl::pair<K,V>& pair : tmp) {
			insert(pair.first, pair.second);
		}
	}
	
	bool find(const K& key, entry_t**& p_row, vnl::pair<K,V>*& value) {
		int index = std::hash<K>{}(key) % N;
		p_row = &table[index];
		while(true) {
			entry_t* row = *p_row;
			if(!row) {
				break;
			}
			if(row->pair.first == key) {
				value = &row->pair;
				return true;
			}
			p_row = &row->next;
		}
		return false;
	}
	
	Allocator<TPage> memory;
	
private:
	table_t table;
	entry_t* p_front;
	int N;
	int count;
	
};


}

#endif /* INCLUDE_PHY_MAP_H_ */