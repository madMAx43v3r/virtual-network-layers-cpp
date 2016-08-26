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
void sort(Iter first, Iter last) {
	if(first != last) {
		while(true) {
			Iter prev = first;
			Iter curr = first;
			curr++;
			bool pass = true;
			while(curr != last) {
				if(*curr < *prev) {
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
void sort_desc(Iter first, Iter last) {
	if(first != last) {
		while(true) {
			Iter prev = first;
			Iter curr = first;
			curr++;
			bool pass = true;
			while(curr != last) {
				if(*curr > *prev) {
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
void sort_ptr(Iter first, Iter last) {
	if(first != last) {
		while(true) {
			Iter prev = first;
			Iter curr = first;
			curr++;
			bool pass = true;
			while(curr != last) {
				if(**curr < **prev) {
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
void sort_desc_ptr(Iter first, Iter last) {
	if(first != last) {
		while(true) {
			Iter prev = first;
			Iter curr = first;
			curr++;
			bool pass = true;
			while(curr != last) {
				if(**curr > **prev) {
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


template<class Iter, class T>
Iter find(Iter first, Iter last, T obj) {
	Iter iter = first;
	while(iter != last) {
		if(*iter == obj) {
			return iter;
		}
		iter++;
	}
	return last;
}



}

#endif /* ALGORITHM_H_ */
