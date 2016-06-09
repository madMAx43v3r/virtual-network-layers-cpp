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

#include "vnl/Util.h"


namespace vnl {

class Address {
public:
	
	uint64_t A;
	uint64_t B;
	
	Address() {
		A = 0;
		B = 0;
	}
	
	Address(uint64_t A_, uint64_t B_) {
		A = A_;
		B = B_;
	}
	
	uint64_t domain() const {
		return A;
	}
	
	uint64_t topic() const {
		return B;
	}
	
	friend std::ostream& operator<<(std::ostream& stream, const Address& addr) {
		auto state = stream.flags(std::ios::hex);
		stream << addr.A << ":" << addr.B;
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
