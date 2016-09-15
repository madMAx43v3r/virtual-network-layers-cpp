/*
 * Hash32.h
 *
 *  Created on: Sep 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_HASH32_H_
#define INCLUDE_VNL_HASH32_H_

#include <vnl/HashValue32Support.hxx>
#include <vnl/Hash.h>


namespace vnl {

class HashValue32 : public Hash32, public HashValue32Base {
public:
	HashValue32() : Hash32((uint32_t)0) {}
	HashValue32(Hash32 hash) : Hash32(hash) {}
	
	operator uint32_t() const {
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
		out.writeInt(value);
	}
	
	void deserialize(vnl::io::TypeInput& in, int size) {
		in.readInt(value);
	}
	
};


}

#endif /* INCLUDE_VNL_HASH32_H_ */
