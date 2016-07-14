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
 * This is a list for use on small data types.
 */
template<typename T>
class List : public Queue<T> {
public:
	
	T& push_back() {
		return push_back(T());
	}
	
	T& push_back(const T& obj) {
		return Queue<T>::push(obj);
	}
	
	bool pop_front(T& obj) {
		return Queue<T>::pop(obj);
	}
	
};



}


#endif /* INCLUDE_PHY_LIST_H_ */
