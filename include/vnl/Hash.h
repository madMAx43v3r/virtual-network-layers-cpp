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
#include <vnl/Address.h>


namespace vnl {

	class Var;
	uint64_t hash(const Var& obj);
	
	inline uint64_t hash(const int8_t& val) { return val; }
	inline uint64_t hash(const int16_t& val) { return val; }
	inline uint64_t hash(const int32_t& val) { return val; }
	inline uint64_t hash(const int64_t& val) { return val; }
	
	inline uint64_t hash(const uint8_t& val) { return val; }
	inline uint64_t hash(const uint16_t& val) { return val; }
	inline uint64_t hash(const uint32_t& val) { return val; }
	inline uint64_t hash(const uint64_t& val) { return val; }
	
	inline uint64_t hash(const float& val) { return *((const uint32_t*)&val); }
	inline uint64_t hash(const double& val) { return *((const uint64_t*)&val); }
	
	inline uint64_t hash(const Hash32& val) {
		return val.value;
	}
	
	inline uint64_t hash(const Hash64& val) {
		return val.value;
	}
	
	inline uint64_t hash(const Address& val) {
		return val.A xor val.B;
	}
	
	inline uint64_t hash(const String& val) {
		return hash64(val);
	}
	
	template<typename T>
	uint64_t hash(const T* p_val) {
		if(p_val) {
			return hash(*p_val);
		}
		return 0;
	}
	
	
} // vnl

#endif /* INCLUDE_VNL_HASH_H_ */
