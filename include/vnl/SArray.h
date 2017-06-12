/*
 * SArray.h
 *
 *  Created on: Feb 4, 2017
 *      Author: mwittal
 */

#ifndef INCLUDE_VNL_SARRAY_H_
#define INCLUDE_VNL_SARRAY_H_

#include <vnl/Array.h>


namespace vnl {

template<typename T>
using SArray = Array<T, vnl::Block>;


} // vnl

#endif /* INCLUDE_VNL_SARRAY_H_ */
