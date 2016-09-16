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
	
	HashValue32& operator=(const Hash32& hash) {
		value = hash;
		return *this;
	}
	
	void to_string_ex(vnl::String& str) const {
		str << vnl::hex(value);
	}
	
	void from_string(const vnl::String& str) {
		// TODO
		assert(false);
	}
	
	void serialize(vnl::io::TypeOutput& out) const {
		out.putEntry(VNL_IO_INTEGER, VNL_IO_DWORD);
		out.writeInt(value);
	}
	
	void deserialize(vnl::io::TypeInput& in, int size) {
		int id = in.getEntry(size);
		if(id == VNL_IO_INTEGER && size == VNL_IO_DWORD) {
			in.readInt(value);
		} else {
			in.skip(id, size);
		}
	}
	
};


}

#endif /* INCLUDE_VNL_HASH32_H_ */
