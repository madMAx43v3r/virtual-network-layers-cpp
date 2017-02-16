/*
 * Hash32.h
 *
 *  Created on: Sep 15, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_VNL_HASH32_H_
#define INCLUDE_VNL_HASH32_H_

#include <vnl/io.h>
#include <vnl/Util.h>
#include <vnl/String.h>


namespace vnl {

class Hash32 {
public:
	Hash32() : value(0) {}
	Hash32(int16_t hash) : value(0) {}
	Hash32(int32_t hash) : value(hash) {}
	Hash32(int64_t hash) : value(hash) {}
	Hash32(uint16_t hash) : value(0) {}
	Hash32(uint32_t hash) : value(hash) {}
	Hash32(uint64_t hash) : value(hash) {}
	
	Hash32(const char* str) : value(hash64(str)) {}
	Hash32(const std::string& str) : value(hash64(str)) {}
	Hash32(const vnl::String& str) : value(hash64(str)) {}
	
	operator uint32_t() const {
		return value;
	}
	
	bool operator==(const char* str) const {
		return value == (uint32_t)hash64(str);
	}
	
	bool operator==(const vnl::String& str) const {
		return value == (uint32_t)hash64(str);
	}
	
	bool operator!=(const char* str) const {
		return value != (uint32_t)hash64(str);
	}
	
	bool operator!=(const vnl::String& str) const {
		return value != (uint32_t)hash64(str);
	}
	
	uint32_t value;
	
};


inline void read(vnl::io::TypeInput& in, vnl::Hash32& val) {
	int size = 0;
	int id = in.getEntry(size);
	if(id == VNL_IO_INTEGER) {
		if(size == VNL_IO_DWORD) {
			uint32_t tmp;
			in.readInt(tmp);
			val = tmp;
		} else if(size == VNL_IO_QWORD) {
			uint64_t tmp;
			in.readLong(tmp);
			val = tmp;
		} else {
			in.skip(id, size);
		}
	} else {
		in.skip(id, size);
	}
}

inline void write(vnl::io::TypeOutput& out, const vnl::Hash32& val) {
	out.putEntry(VNL_IO_INTEGER, VNL_IO_DWORD);
	out.writeInt(val);
}

inline void to_string(vnl::String& str, const vnl::Hash32& val) {
	str << val.value;
}

inline void from_string(const vnl::String& str, vnl::Hash32& val) {
	if(!str.empty() && str.front() == '"') {
		// TODO
	} else {
		val.value = vnl::atoull(str);
	}
}


} // vnl

#endif /* INCLUDE_VNL_HASH32_H_ */
