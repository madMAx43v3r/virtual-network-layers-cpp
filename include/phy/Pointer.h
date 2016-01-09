/*
 * Pointer.h
 *
 *  Created on: Jan 8, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_PHY_POINTER_H_
#define INCLUDE_PHY_POINTER_H_

#include "Object.h"

namespace vnl { namespace phy {

template<typename T>
class Pointer {
public:
	Pointer() : obj(0) {}
	
	Pointer(Object* obj) : obj(0) {
		*this = obj;
	}
	
	Pointer& operator=(Object* obj) {
		if(obj->oid == T::id) {
			this->obj = obj;
		}
		return *this;
	}
	
	Object* operator->() const {
		return obj;
	}
	
	Object& operator*() {
		return *obj;
	}
	
	const Object& operator*() const {
		return *obj;
	}
	
protected:
	Object* obj;
	
};


}}

#endif /* INCLUDE_PHY_POINTER_H_ */
