/*
 * Operators.h
 *
 *  Created on: Jan 5, 2017
 *      Author: mad
 */

#ifndef INCLUDE_VNL_OPERATORS_H_
#define INCLUDE_VNL_OPERATORS_H_


namespace vnl {
	
	template<typename T>
	bool equals(const T& A, const T& B) {
		return A == B;
	}
	
	template<typename T>
	bool equals(const T* A, const T* B) {
		if(A && B) {
			return *A == *B;
		}
		return A == B;
	}
	
} // vnl

#endif /* INCLUDE_VNL_OPERATORS_H_ */
