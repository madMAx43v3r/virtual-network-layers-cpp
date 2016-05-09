/*
 * pool.h
 *
 *  Created on: Mar 6, 2016
 *      Author: mad
 */

#ifndef INCLUDE_UTIL_POOL_H_
#define INCLUDE_UTIL_POOL_H_

#include "util/spinlock.h"
#include "Queue.h"
#include "AtomicQueue.h"


namespace vnl { namespace phy {

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


template<typename T>
class AtomicPool {
public:
	AtomicPool(Region& mem) : mem(mem), list(mem) {}
	
	AtomicPool(const AtomicPool&) = delete;
	AtomicPool& operator=(const AtomicPool&) = delete;
	
	T* create() {
		T* obj;
		if(list.pop(obj)) {
			obj = new(obj) T();
		} else {
			sync.lock();
			obj = mem.create<T>();
			sync.unlock();
		}
		return obj;
	}
	
	void destroy(T* obj) {
		obj->~T();
		list.push(obj);
	}
	
protected:
	Region& mem;
	AtomicQueue<T*> list;
	vnl::util::spinlock sync;
	
};


}}

#endif /* INCLUDE_UTIL_POOL_H_ */
