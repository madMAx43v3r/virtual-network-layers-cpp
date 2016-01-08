/*
 * Address.h
 *
 *  Created on: 16.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_ADDRESS_H_
#define INCLUDE_ADDRESS_H_

#include <stdint.h>
#include <string>
#include <sstream>

#include "Util.h"

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
	
	Address(const std::string& A_, const std::string& B_) {
		A = Util::hash64(A_);
		B = Util::hash64(B_);
	}
	
	std::string toString() {
		std::stringstream ss;
		ss << std::hex << A << ":" << B;
		return ss.str();
	}
	
};

inline bool operator<(const Address& A, const Address& B) {
	return (A.A xor A.B) < (B.A xor B.B);
}

}

#endif /* INCLUDE_ADDRESS_H_ */
