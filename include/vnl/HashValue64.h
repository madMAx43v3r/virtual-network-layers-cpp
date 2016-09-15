/*
 * Hash64.h
 *
 *  Created on: Sep 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_HASH64_H_
#define INCLUDE_VNL_HASH64_H_

#include <vnl/HashValue64Support.hxx>
#include <vnl/Hash.h>


namespace vnl {

class HashValue64 : public Hash64, public HashValue64Base {
public:
	HashValue64() : Hash64((uint64_t)0) {}
	HashValue64(Hash64 hash) : Hash64(hash) {}
	
	operator uint64_t() const {
		return value;
	}
	
	void to_string_ex(vnl::String& str) const {
		str << vnl::hex(value);
	}
	
	void from_string(const vnl::String& str) {
		// TODO
		assert(false);
	}
	
	void serialize(vnl::io::TypeOutput& out) const {
		out.writeLong(value);
	}
	
	void deserialize(vnl::io::TypeInput& in, int size) {
		in.readLong(value);
	}
	
};


}

#endif /* INCLUDE_VNL_HASH64_H_ */
