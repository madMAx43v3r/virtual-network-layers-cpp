/*
 * Pointer.h
 *
 *  Created on: Jul 11, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_POINTER_H_
#define CPP_INCLUDE_VNI_POINTER_H_

#include <vnl/io.h>


namespace vnl {

template<typename T>
class Pointer : public vnl::io::Serializable {
public:
	Pointer() : ptr(0) {}
	
	Pointer(T* obj) : ptr(obj) {}
	
	Pointer(const Pointer& other) : ptr(0) {
		ptr = other.clone();
	}
	
	virtual ~Pointer() {
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
	
	T* value() {
		return ptr;
	}
	
	T* release() {
		T* ret = ptr;
		ptr = 0;
		return ret;
	}
	
	T* clone() const {
		if(ptr) {
			return ptr->clone();
		} else {
			return 0;
		}
	}
	
	void to_string_ex(vnl::String& str) const {
		str << "{\"T\": \"";
		if(ptr) {
			str << ptr->type_name();
		}
		str << "\", \"V\": ";
		vnl::to_string(str, ptr);
		str << "}";
	}
	
	void from_string(const vnl::String& str) {
		// TODO
		assert(false);
	}
	
	void serialize(vnl::io::TypeOutput& out) const {
		vnl::write(out, ptr);
	}
	
	void deserialize(vnl::io::TypeInput& in, int size) {
		destroy();
		Value* value = vnl::read(in);
		if(value) {
			ptr = dynamic_cast<T*>(value);
			if(!ptr) {
				vnl::destroy(value);
			}
		}
	}
	
protected:
	void destroy() {
		vnl::destroy<T>(ptr);
	}
	
private:
	T* ptr;
	
};


template<typename T>
void read(vnl::io::TypeInput& in, Pointer<T>& obj) {
	obj.deserialize(in, 0);
}

template<typename T>
void write(vnl::io::TypeOutput& out, const Pointer<T>& obj) {
	obj.serialize(out);
}

template<typename T>
void to_string(vnl::String& str, const Pointer<T>& obj) {
	obj.to_string_ex(str);
}

template<typename T>
void from_string(const vnl::String& str, Pointer<T>& obj) {
	obj.from_string(str);
}


} // vnl

#endif /* CPP_INCLUDE_VNI_POINTER_H_ */
