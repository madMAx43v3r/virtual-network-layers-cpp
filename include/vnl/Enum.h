/*
 * Enum.h
 *
 *  Created on: Jul 29, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_ENUM_H_
#define INCLUDE_VNL_ENUM_H_

#include <vnl/Interface.h>

namespace vnl {

class Enum : public Interface {
public:
	Enum() : value(0) {}
	
	operator uint32_t() const {
		return value;
	}
	
	Enum& operator=(const uint32_t& val) {
		value = val;
		return *this;
	}
	
	virtual void serialize(vnl::io::TypeOutput& out) const {
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_BEGIN);
		out.putHash(vni_hash());
		out.putEntry(VNL_IO_CALL, 0);
		out.putHash(value);
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_END);
	}
	
	virtual void from_string(vnl::io::ByteInput& in) {
		// TODO
	}
	
protected:
	virtual bool vni_call(vnl::io::TypeInput& in, uint32_t hash, int num_args) {
		if(num_args == 0) {
			value = hash;
			return true;
		}
		return false;
	}
	
	uint32_t value;
	
};


}

#endif /* INCLUDE_VNL_ENUM_H_ */
