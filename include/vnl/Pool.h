/*
 * pool.h
 *
 *  Created on: Mar 6, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_POOL_H_
#define INCLUDE_VNL_POOL_H_

#include <vnl/Queue.h>
#include <vnl/Map.h>


namespace vnl {

template<typename T>
class Pool {
public:
	Pool() {}
	
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
	PageAllocator memory;
	Queue<T*> list;
	
};


class GenericPool {
public:
	GenericPool() {}
	
	GenericPool(const GenericPool&) = delete;
	GenericPool& operator=(const GenericPool&) = delete;
	
	template<typename T>
	T* create() {
		Queue<void*>& list = table[sizeof(T)];
		void* obj;
		if(list.pop(obj)) {
			return new(obj) T();
		} else {
			return memory.create<T>();
		}
	}
	
	template<typename T>
	void destroy(T* obj, int size) {
		Queue<void*>& list = table[size];
		obj->~T();
		list.push(obj);
	}
	
protected:
	PageAllocator memory;
	Map<int, Queue<void*> > table;
	
};


class MessagePool : public GenericPool {
public:
	template<typename T>
	T* create() {
		T* msg = GenericPool::create<T>();
		msg->buffer = this;
		msg->msg_size = sizeof(T);
		return msg;
	}
	
};



} // vnl

#endif /* INCLUDE_VNL_POOL_H_ */
