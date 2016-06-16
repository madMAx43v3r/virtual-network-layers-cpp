/*
 * List.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_LIST_H_
#define INCLUDE_PHY_LIST_H_

#include "Queue.h"


namespace vnl {

/*
 * This is a list.
 * Maximum element size at default is 30 bytes.
 */
template<typename T, int N = 8>
class List : public Queue<T,N> {
public:
	T& push_back(const T& obj) {
		return Queue<T,N>::push(obj);
	}
	
	bool pop_front(T& obj) {
		return Queue<T,N>::pop(obj);
	}
	
};



}


#endif /* INCLUDE_PHY_LIST_H_ */
