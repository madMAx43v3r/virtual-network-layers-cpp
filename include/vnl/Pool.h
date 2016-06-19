/*
 * pool.h
 *
 *  Created on: Mar 6, 2016
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_POOL_H_
#define INCLUDE_UTIL_POOL_H_

#include "vnl/Queue.h"
#include "vnl/util/spinlock.h"


namespace vnl {

template<typename T>
class Pool {
public:
	Pool(const Pool&) = delete;
	Pool& operator=(const Pool&) = delete;
	
	T* create() {
		T* obj;
		if(list.pop(obj)) {
			return new(obj) T();
		} else {
			return memory.create<T>();
		}
	}
	
	void destroy(T* obj) {
		obj->~T();
		list.push(obj);
	}
	
protected:
	PageAlloc memory;
	Queue<T*> list;
	
};


template<typename T>
class GlobalPool {
public:
	static T* create() {
		sync.lock();
		T* obj = pool.create();
		sync.unlock();
		return obj;
	}
	
	static void destroy(T* obj) {
		sync.lock();
		pool.destroy(obj);
		sync.unlock();
	}
	
private:
	static util::spinlock sync;
	static Pool<T> pool;
};

template<typename T> util::spinlock GlobalPool<T>::sync;
template<typename T> Pool<T> GlobalPool<T>::pool;


template<typename T>
T* create() {
	return GlobalPool<T>::create();
}

template<typename T>
void destroy(T*& obj) {
	GlobalPool<T>::destroy(obj);
	obj = 0;
}




} // vnl

#endif /* INCLUDE_UTIL_POOL_H_ */
