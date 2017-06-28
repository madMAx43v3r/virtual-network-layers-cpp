/*
 * Pool.h
 *
 *  Created on: Mar 6, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_POOL_H_
#define INCLUDE_VNL_POOL_H_

#include <mutex>

#include <vnl/Memory.h>
#include <vnl/Queue.h>


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
		memset(obj, 0, sizeof(T));
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
		if(size <= VNL_BLOCK_SIZE - sizeof(void*)) {
			return alloc_ex<Block>();
		} else if(size <= VNL_PAGE_SIZE - sizeof(void*)) {
			return alloc_ex<Page>();
		}
		assert(false);
		return 0;
	}
	
	template<typename T>
	T* create() {
		return new(alloc(sizeof(T))) T();
	}
	
	template<typename T>
	void destroy(T* obj, int size) {
		obj->~T();
		push_back(obj, size);
	}
	
	template<typename T>
	void push_back(T* obj, int size) {
		if(size <= VNL_BLOCK_SIZE - sizeof(void*)) {
			destroy_ex(*(((Block**)obj) - 1));
		} else if(size <= VNL_PAGE_SIZE - sizeof(void*)) {
			destroy_ex(*(((Page**)obj) - 1));
		}
	}
	
protected:
	template<typename T>
	void* alloc_ex() {
		T* page = T::alloc();
		page->user = T::size;
		page->template type_at<void*>(0) = page;
		return page->mem + sizeof(void*);
	}
	
	template<typename T>
	void destroy_ex(T* page) {
		assert(page->user == T::size);
		page->free();
	}
	
};


} // vnl

#endif /* INCLUDE_VNL_POOL_H_ */
