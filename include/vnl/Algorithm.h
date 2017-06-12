/*
 * Algorithm.h
 *
 *  Created on: Jun 8, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_ALGORITHM_H_
#define INCLUDE_VNL_ALGORITHM_H_

#include <vnl/List.h>


namespace vnl {

template<class Iter>
void sort(Iter begin, Iter end, bool asc = true) {
	List<typename Iter::value_type> list;
	for(Iter it = begin; it != end; ++it) {
		bool found = false;
		// fill list in opposite order (much faster in case input is already sorted)
		for(auto it2 = list.begin(); it2 != list.end(); ++it2) {
			if(*it < *it2 != asc) {
				list.insert(it2, *it);
				found = true;
				break;
			}
		}
		if(!found) {
			list.push_back(*it);
		}
	}
	// now loop over list backwards to get correct order
	Iter it = begin;
	auto it2 = list.end();
	while(it2 != list.begin()) {
		*(it++) = *(--it2);
	}
}

template<class Iter>
void sort_desc(Iter begin, Iter end) {
	sort(begin, end, false);
}


template<class Iter, class T>
Iter find(Iter begin, Iter end, T obj) {
	Iter iter = begin;
	while(iter != end) {
		if(*iter == obj) {
			return iter;
		}
		iter++;
	}
	return end;
}


} // vnl

#endif /* INCLUDE_VNL_ALGORITHM_H_ */
