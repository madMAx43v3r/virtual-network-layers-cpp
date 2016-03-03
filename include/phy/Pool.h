/*
 * Custom.h
 *
 *  Created on: Mar 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_POOL_H_
#define INCLUDE_PHY_POOL_H_

#include <malloc.h>
#include <vector>

namespace vnl { namespace phy {

template<typename T>
class Pool {
public:
	~Pool() {
		for(T* obj : list) {
			::free(obj);
		}
	}
	
	T* alloc() {
		if(list.size()) {
			void* obj = list.back();
			list.pop_back();
			return new(obj) T();
		} else {
			return new(malloc(sizeof(T))) T();
		}
	}
	
	void free(T* obj) {
		obj->~T();
		list.push_back(obj);
	}
	
protected:
	std::vector<T*> list;
	
};


}}

#endif /* INCLUDE_PHY_POOL_H_ */
