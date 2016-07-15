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

template<typename T, class TPage = Page>
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
			return memory.template create<T>();
		}
	}
	
	void destroy(T* obj) {
		obj->~T();
		list.push(obj);
	}
	
protected:
	Allocator<TPage> memory;
	Queue<T*> list;
	
};


class GlobalPool {
public:
	GlobalPool() {}
	
	GlobalPool(const GlobalPool&) = delete;
	GlobalPool& operator=(const GlobalPool&) = delete;
	
	void* alloc(int size) {
		assert(size <= VNL_PAGE_SIZE);
		sync.lock();
		void* obj;
		if(!table[size].pop(obj)) {
			obj = memory.alloc(size);
		}
		sync.unlock();
		return obj;
	}
	
	template<typename T>
	T* create() {
		return new(alloc(sizeof(T))) T();
	}
	
	template<typename T>
	void push_back(T* obj, int size) {
		sync.lock();
		table[size].push(obj);
		sync.unlock();
	}
	
protected:
	vnl::PageAllocator memory;
	vnl::Queue<void*> table[VNL_PAGE_SIZE];
	vnl::util::spinlock sync;
	
};

extern GlobalPool* global_pool;


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
