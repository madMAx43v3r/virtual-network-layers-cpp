/*
 * Pointer.h
 *
 *  Created on: Jul 11, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_POINTER_H_
#define CPP_INCLUDE_VNI_POINTER_H_

#include <vnl/PointerSupport.hxx>


namespace vnl {

template<class T>
class Pointer : public PointerBase<T> {
public:
	Pointer() : ptr(0) {}
	
	Pointer(T* obj) : ptr(obj) {}
	
	Pointer(const Pointer& other) : ptr(0) {
		ptr = other.clone();
	}
	
	~Pointer() {
		destroy();
	}
	
	Pointer& operator=(T* obj) {
		destroy();
		ptr = obj;
		return *this;
	}
	
	Pointer& operator=(const Pointer& other) {
		destroy();
		ptr = other.clone();
		return *this;
	}
	
	operator bool() const {
		return !is_null();
	}
	
	bool is_null() const {
		return ptr == 0;
	}
	
	T& operator*() {
		return *ptr;
	}
	
	const T& operator*() const {
		return *ptr;
	}
	
	T& operator->() {
		return *ptr;
	}
	
	const T& operator->() const {
		return *ptr;
	}
	
	T* clone() const {
		if(ptr) {
			return ptr->clone();
		} else {
			return 0;
		}
	}
	
	void serialize(vnl::io::TypeOutput& out) const {
		if(ptr) {
			vnl::write(out, ptr);
		} else {
			out.putNull();
		}
	}
	
	void deserialize(vnl::io::TypeInput& in, int size) {
		int id = in.getEntry(size);
		if(id == VNL_IO_CLASS) {
			uint32_t hash;
			in.getHash(hash);
			Value* value = vnl::create(hash);
			if(value) {
				ptr = dynamic_cast<T*>(value);
				if(!ptr) {
					vnl::destroy(value);
				}
			} else {
				in.skip(id, size, hash);
			}
		} else {
			in.skip(id, size);
		}
	}
	
protected:
	void destroy() {
		vnl::destroy<T>(ptr);
	}
	
private:
	T* ptr;
	
};




}

#endif /* CPP_INCLUDE_VNI_POINTER_H_ */
