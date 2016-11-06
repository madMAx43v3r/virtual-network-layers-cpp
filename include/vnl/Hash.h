/*
 * Hash.h
 *
 *  Created on: Jul 6, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_HASH_H_
#define INCLUDE_VNL_HASH_H_

#include <vnl/Hash32.h>
#include <vnl/Hash64.h>

namespace std {
	
	template<>
	struct hash<vnl::String> {
		size_t operator()(const vnl::String& str) const {
			return vnl::hash64(str);
		}
	};
	
}

#endif /* INCLUDE_VNL_HASH_H_ */
