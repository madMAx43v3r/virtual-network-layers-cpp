/*
 * pool.h
 *
 *  Created on: Mar 6, 2016
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_POOL_H_
#define INCLUDE_UTIL_POOL_H_

#include "vnl/Queue.h"


namespace vnl {

template<typename T>
class Pool {
public:
	Pool(Region& mem) : mem(mem), list(mem) {}
	
	Pool(const Pool&) = delete;
	Pool& operator=(const Pool&) = delete;
	
	T* create() {
		T* obj;
		if(list.pop(obj)) {
			return new(obj) T();
		} else {
			return mem.create<T>();
		}
	}
	
	void destroy(T* obj) {
		obj->~T();
		list.push(obj);
	}
	
protected:
	Region& mem;
	Queue<T*> list;
	
};


}

#endif /* INCLUDE_UTIL_POOL_H_ */
