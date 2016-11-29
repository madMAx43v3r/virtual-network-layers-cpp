/*
 * Type.h
 *
 *  Created on: Jun 27, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNI_TYPE_H_
#define INCLUDE_VNI_TYPE_H_

#include <vnl/io.h>
#include <vnl/Hash.h>
#include <vnl/Vector.h>
#include <vnl/String.h>
#include <vnl/Binary.h>
#include <vnl/Array.h>
#include <vnl/List.h>
#include <vnl/Map.h>
#include <vnl/Pool.h>


namespace vnl {

class Type : public vnl::io::Serializable {
public:
	Type() {}
	
	virtual ~Type() {}
	
	virtual uint32_t vni_hash() const = 0;
	virtual const char* type_name() const = 0;
	
	virtual int num_fields() const = 0;
	virtual int field_index(vnl::Hash32 hash) const = 0;
	virtual const char* field_name(int index) const = 0;
	virtual void get_field(int index, vnl::String& str) const = 0;
	virtual void set_field(int index, const vnl::String& str) = 0;
	
	virtual void from_string(const vnl::String& str) { /* NOT IMPLEMENTED */ }
	
	virtual void to_string_ex(vnl::String& str) const {
		str << "{";
		int N = num_fields();
		for(int i = 0; i < N; ++i) {
			if(i > 0) {
				str << ", ";
			}
			str << "\"" << field_name(i) << "\": ";
			get_field(i, str);
		}
		str << "}";
	}
	
	vnl::String to_string() const {
		vnl::String str;
		to_string_ex(str);
		return str;
	}
	
	Type& operator=(const vnl::String& str) {
		from_string(str);
		return *this;
	}
	
};


} // vnl

#endif /* INCLUDE_VNI_TYPE_H_ */
