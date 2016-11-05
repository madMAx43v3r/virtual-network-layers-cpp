/*
 * Vector.h
 *
 *  Created on: Jul 6, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_VECTOR_H_
#define INCLUDE_VNL_VECTOR_H_


namespace vnl {

template<typename T, int N>
class Vector {
public:
	Vector() {
		for(int i = 0; i < N; ++i) {
			data[i] = T();
		}
	}
	
	T& operator[](int i) {
		return data[i];
	}
	
	const T& operator[](int i) const {
		return data[i];
	}
	
	operator T*() {
		return &data[0];
	}
	
	operator const T*() {
		return &data[0];
	}
	
	int size() const {
		return N;
	}
	
private:
	T data[N];
	
};


}

#endif /* INCLUDE_VNL_VECTOR_H_ */
