/*
 * Enum.h
 *
 *  Created on: Jul 29, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_ENUM_H_
#define INCLUDE_VNL_ENUM_H_

#include <vnl/Interface.h>
#include <vnl/Type.hxx>

namespace vnl {

class Enum : public Interface {
public:
	Enum() {}
	
	virtual void serialize(vnl::io::TypeOutput& out) const {
		vnl::write(out, value);
	}
	
	virtual void deserialize(vnl::io::TypeInput& in, int size) {
		vnl::read(in, value);
	}
	
	virtual void from_string(const vnl::String& str) {
		value = str;
	}
	
protected:
	vnl::Hash32 value;
	
};


} // vnl

#endif /* INCLUDE_VNL_ENUM_H_ */
