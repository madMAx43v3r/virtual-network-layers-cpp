/*
 * Hash32.h
 *
 *  Created on: Sep 15, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_VNL_HASH32_H_
#define INCLUDE_VNL_HASH32_H_

#include <string>
#include <algorithm>

#include <vnl/Util.h>
#include <vnl/String.h>


namespace vnl {

class Hash32 {
public:
	Hash32() : value(0) {}
	Hash32(uint16_t hash) : value(0) {}
	Hash32(uint32_t hash) : value(hash) {}
	Hash32(uint64_t hash) : value(hash) {}
	
	Hash32(const char* str) : value(hash64(str)) {}
	Hash32(const std::string& str) : value(hash64(str)) {}
	Hash32(const vnl::String& str) : value(hash64(str)) {}
	
	operator uint32_t() const {
		return value;
	}
	
	uint32_t value;
	
};


} // vnl

namespace std {

	template<>
	struct hash<vnl::Hash32> {
		size_t operator()(const vnl::Hash32& x) const {
			return x;
		}
	};
	
}

#endif /* INCLUDE_VNL_HASH32_H_ */
