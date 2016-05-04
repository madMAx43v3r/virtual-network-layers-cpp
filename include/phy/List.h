/*
 * List.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_LIST_H_
#define INCLUDE_PHY_LIST_H_

#include "phy/Queue.h"


namespace vnl { namespace phy {

/*
 * This is a list.
 * Maximum element size at default is 500 bytes.
 */
template<typename T, int N = 8>
class List : public Queue<T,N> {
public:
	List(Region* mem) : Queue(mem) {}
	
	List(const List&) = delete;
	
	T& push_back(const T& obj) {
		return push(obj);
	}
	
	bool pop_front(T& obj) {
		return pop(obj);
	}
	
};



}}


#endif /* INCLUDE_PHY_LIST_H_ */
