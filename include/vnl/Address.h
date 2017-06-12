/*
 * Address.h
 *
 *  Created on: 16.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_VNL_ADDRESS_H_
#define INCLUDE_VNL_ADDRESS_H_

#include <ostream>

#include <vnl/Hash64.h>
#include <vnl/io.h>


namespace vnl {

class Address {
public:
	
	uint64_t A;
	uint64_t B;
	
	Address() {
		A = 0;
		B = 0;
	}
	
	Address(Hash64 A_, Hash64 B_) {
		A = A_;
		B = B_;
	}
	
	Hash64 domain() const {
		return A;
	}
	
	Hash64 topic() const {
		return B;
	}
	
	bool is_null() {
		return A == 0 && B == 0;
	}
	
	friend vnl::String& operator<<(vnl::String& stream, const Address& addr) {
		stream << hex(addr.A) << ":" << hex(addr.B);
		return stream;
	}
	
	friend std::ostream& operator<<(std::ostream& stream, const Address& addr) {
		auto state = stream.flags(std::ios::hex);
		stream << "0x" << addr.A << ":0x" << addr.B;
		stream.flags(state);
		return stream;
	}
	
};


inline bool operator<(const Address& A, const Address& B) {
	return (A.A xor A.B) < (B.A xor B.B);
}

inline bool operator==(const Address& A, const Address& B) {
	return A.A == B.A && A.B == B.B;
}

inline bool operator!=(const Address& A, const Address& B) {
	return A.A != B.A || A.B != B.B;
}

inline void read(vnl::io::TypeInput& in, Address& val) {
	int size = 0;
	int id = in.getEntry(size);
	if(id == VNL_IO_ARRAY && size == 2) {
		Hash64 A, B;
		vnl::read(in, A);
		vnl::read(in, B);
		val = Address(A, B);
	} else {
		in.skip(id, size);
	}
}

inline void write(vnl::io::TypeOutput& out, const Address& val) {
	out.putEntry(VNL_IO_ARRAY, 2);
	vnl::write(out, Hash64(val.A));
	vnl::write(out, Hash64(val.B));
}

inline void to_string(vnl::String& str, const Address& val) {
	str << "[" << val.A << ", " << val.B << "]";
}

inline void from_string(const vnl::String& str, Address& val) {
	/* TODO */
	assert(false);
}


} // vnl

#endif /* INCLUDE_VNL_ADDRESS_H_ */
