/*
 * Hash64.h
 *
 *  Created on: Sep 15, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_VNL_HASH64_H_
#define INCLUDE_VNL_HASH64_H_

#include <string>
#include <algorithm>

#include <vnl/Util.h>
#include <vnl/String.h>


namespace vnl {

class Hash64 {
public:
	Hash64() : value(0) {}
	Hash64(uint32_t hash) : value(0) {}
	Hash64(uint64_t hash) : value(hash) {}
	
	Hash64(const char* str) : value(hash64(str)) {}
	Hash64(const std::string& str) : value(hash64(str)) {}
	Hash64(const vnl::String& str) : value(hash64(str)) {}
	
	operator uint64_t() const {
		return value;
	}
	
	bool operator==(const char* str) const {
		return value == hash64(str);
	}
	
	bool operator==(const vnl::String& str) const {
		return value == hash64(str);
	}
	
	bool operator!=(const char* str) const {
		return value != hash64(str);
	}
	
	bool operator!=(const vnl::String& str) const {
		return value != hash64(str);
	}
	
	uint64_t value;
	
};

} // vnl

namespace std {

	template<>
	struct hash<vnl::Hash64> {
		size_t operator()(const vnl::Hash64& x) const {
			return x;
		}
	};
	
}

#endif /* INCLUDE_VNL_HASH64_H_ */
