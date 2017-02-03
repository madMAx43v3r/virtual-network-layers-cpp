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
	Enum() {}
	
	void serialize(vnl::io::TypeOutput& out) const;
	
	void deserialize(vnl::io::TypeInput& in, int size);
	
	void from_string(const vnl::String& str);
	
protected:
	vnl::Hash32 value;
	
};


} // vnl

#endif /* INCLUDE_VNL_ENUM_H_ */
