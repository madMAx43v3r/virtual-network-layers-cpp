/*
 * Address.h
 *
 *  Created on: 16.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_ADDRESS_H_
#define INCLUDE_ADDRESS_H_

#include <stdint.h>
#include <ostream>

#include <vnl/Hash.h>
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
	
	uint64_t domain() const {
		return A;
	}
	
	uint64_t mac() const {
		return B;
	}
	
	void serialize(vnl::io::TypeOutput& out) const {
		out.writeLong(A);
		out.writeLong(B);
	}
	
	void deserialize(vnl::io::TypeInput& in, int size) {
		int64_t a = 0;
		int64_t b = 0;
		in.readLong(a);
		in.readLong(b);
		A = a;
		B = b;
	}
	
	friend std::ostream& operator<<(std::ostream& stream, const Address& addr) {
		auto state = stream.flags(std::ios::hex);
		stream << addr.A << ":" << addr.B;
		stream.flags(state);
		return stream;
	}
	
};


class Endpoint : public Address {
public:
	Endpoint(const String& domain, const String& mac)
		:	domain(domain), mac(mac)
	{
		A = hash64(domain);
		B = hash64(mac);
	}
	
	String domain;
	String mac;
	
};


inline bool operator<(const Address& A, const Address& B) {
	return (A.A xor A.B) < (B.A xor B.B);
}

inline bool operator==(const Address& A, const Address& B) {
	return A.A == B.A && A.B == B.B;
}

inline static const char* get_local_domain_name() {
	return "vnl/local/domain";
}

inline static uint64_t get_local_domain() {
	return hash64(get_local_domain_name());
}


} // vnl

namespace std {
	template<>
	struct hash<vnl::Address> {
		size_t operator()(const vnl::Address& x) const {
			return x.A xor x.B;
		}
	};
}

#endif /* INCLUDE_ADDRESS_H_ */
