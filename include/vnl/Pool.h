/*
 * Pool.h
 *
 *  Created on: Mar 6, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_POOL_H_
#define INCLUDE_VNL_POOL_H_

#include <mutex>

#include <vnl/Queue.h>
#include <vnl/Map.h>


namespace vnl {

class Message;


template<typename T, typename TPage = Page>
class Pool {
public:
	Pool() {}
	~Pool() {}
	
	Pool(const Pool&) = delete;
	Pool& operator=(const Pool&) = delete;
	
	T* create() {
		T* obj;
		if(free_list.pop(obj)) {
			return new(obj) T();
		} else {
			return memory.template create<T>();
		}
	}
	
	void destroy(T* obj) {
		obj->~T();
#ifdef VNL_MEMORY_DEBUG
		memset(obj, 0, sizeof(T));
#endif
		free_list.push(obj);
	}
	
protected:
	Allocator<TPage> memory;
	Queue<T*> free_list;
	
};


class MessageBuffer {
public:
	virtual ~MessageBuffer() {}
	
	virtual void destroy(Message* obj, int size) = 0;
	
};


template<typename T>
class MessagePool : public Pool<T>, public MessageBuffer {
public:
	T* create() {
		T* msg = Pool<T>::create();
		msg->buffer = this;
		msg->msg_size = sizeof(T);
		return msg;
	}
	
	void destroy(Message* obj, int size) {
		assert(size == sizeof(T));
		Pool<T>::destroy((T*)obj);
	}
	
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
	void destroy(T* obj, int size) {
		obj->~T();
#ifdef VNL_MEMORY_DEBUG
		memset(obj, 0, size);
#endif
		push_back(obj, size);
	}
	
	template<typename T>
	void push_back(T* obj, int size) {
		sync.lock();
		table[size].push(obj);
		sync.unlock();
	}
	
protected:
	Allocator<Page> memory;
	Queue<void*> table[VNL_PAGE_SIZE];
	std::mutex sync;
	
};


} // vnl

#endif /* INCLUDE_VNL_POOL_H_ */
