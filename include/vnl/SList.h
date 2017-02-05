/*
 * SList.h
 *
 *  Created on: Feb 4, 2017
 *      Author: mwittal
 */

#ifndef INCLUDE_VNL_SLIST_H_
#define INCLUDE_VNL_SLIST_H_

#include <vnl/List.h>


namespace vnl {

template<typename T>
using SList = List<T, vnl::Block>;


} // vnl

#endif /* INCLUDE_VNL_SLIST_H_ */
