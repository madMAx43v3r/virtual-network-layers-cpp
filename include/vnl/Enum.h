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
	
	virtual void serialize(vnl::io::TypeOutput& out) const {
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_BEGIN);
		out.putHash(vni_hash());
		out.putEntry(VNL_IO_CALL, 0);
		out.putHash(value);
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_END);
	}
	
	virtual void from_string(const vnl::String& str) {
		// TODO
		assert(false);
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
