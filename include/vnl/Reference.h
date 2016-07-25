/*
 * Reference.h
 *
 *  Created on: Jul 25, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_REFERENCE_H_
#define INCLUDE_VNL_REFERENCE_H_

#include <vnl/Pointer.h>


namespace vnl {

template<typename T>
class Reference {
public:
	Reference() {
		ptr = 0;
	}
	
	Reference(const Pointer<T>& other) {
		ptr = other.get();
	}
	
	Reference& operator=(const Pointer<T>& other) {
		ptr = other.get();
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
	
private:
	T* ptr;
	
};




}

#endif /* INCLUDE_VNL_REFERENCE_H_ */
