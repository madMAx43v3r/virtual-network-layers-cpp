/*
 * Hash.h
 *
 *  Created on: Jul 6, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_HASH_H_
#define INCLUDE_VNL_HASH_H_

#include <vnl/Util.h>


namespace vnl {

class Hash32 {
public:
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


class Hash64 {
public:
	Hash64(uint32_t hash) : value(0) {}
	Hash64(uint64_t hash) : value(hash) {}
	
	Hash64(const char* str) : value(hash64(str)) {}
	Hash64(const std::string& str) : value(hash64(str)) {}
	Hash64(const vnl::String& str) : value(hash64(str)) {}
	
	operator uint64_t() const {
		return value;
	}
	
	uint64_t value;
	
};


}

#endif /* INCLUDE_VNL_HASH_H_ */
