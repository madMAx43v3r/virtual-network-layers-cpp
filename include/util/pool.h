/*
 * pool.h
 *
 *  Created on: Mar 6, 2016
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_POOL_H_
#define INCLUDE_UTIL_POOL_H_

#include <vector>

namespace vnl { namespace util {

template<typename T>
class pool {
public:
	pool() {}
	
	pool(const pool&) = delete;
	pool& operator=(const pool&) = delete;
	
	~pool() {
		for(T* obj : list) {
			delete obj;
		}
	}
	
	T* alloc() {
		if(list.size()) {
			T* obj = list.back();
			list.pop_back();
			obj->~T();
			return new(obj) T();
		} else {
			return new T();
		}
	}
	
	void free(T* obj) {
		list.push_back(obj);
	}
	
protected:
	std::vector<T*> list;
	
};


}}

#endif /* INCLUDE_UTIL_POOL_H_ */
