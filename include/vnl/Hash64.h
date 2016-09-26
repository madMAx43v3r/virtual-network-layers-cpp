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

#include <vnl/io.h>
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


inline void read(vnl::io::TypeInput& in, vnl::Hash64& val) {
	int size = 0;
	int id = in.getEntry(size);
	if(id == VNL_IO_INTEGER && size == VNL_IO_QWORD) {
		uint64_t tmp;
		in.readLong(tmp);
		val = tmp;
	} else {
		in.skip(id, size);
	}
}

inline void write(vnl::io::TypeOutput& out, const vnl::Hash64& val) {
	out.putEntry(VNL_IO_INTEGER, VNL_IO_QWORD);
	out.writeLong(val);
}

inline void to_string(vnl::String& str, const vnl::Hash64& val) {
	str << vnl::hex(val);
}

inline void from_string(const vnl::String& str, vnl::Hash64& val) {
	/* TODO */
	assert(false);
}


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
