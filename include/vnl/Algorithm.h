/*
 * Algorithm.h
 *
 *  Created on: Jun 8, 2016
 *      Author: mad
 */

#ifndef ALGORITHM_H_
#define ALGORITHM_H_


namespace vnl {

template<class Iter>
void sort(Iter first, Iter last, bool asc = true) {
	if(first != last) {
		while(true) {
			Iter prev = first;
			Iter curr = first;
			curr++;
			bool pass = true;
			while(curr != last) {
				if((*curr < *prev) xor asc) {
					std::swap(*prev, *curr);
					pass = false;
				}
				prev = curr;
				curr++;
			}
			if(pass) {
				break;
			}
		}
	}
}

template<class Iter>
void sort_ptr(Iter first, Iter last, bool asc = true) {
	if(first != last) {
		while(true) {
			Iter prev = first;
			Iter curr = first;
			curr++;
			bool pass = true;
			while(curr != last) {
				if((**curr < **prev) xor asc) {
					std::swap(*prev, *curr);
					pass = false;
				}
				prev = curr;
				curr++;
			}
			if(pass) {
				break;
			}
		}
	}
}



}

#endif /* ALGORITHM_H_ */
