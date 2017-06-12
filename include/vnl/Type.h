/*
 * Type.h
 *
 *  Created on: Jun 27, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNI_TYPE_H_
#define INCLUDE_VNI_TYPE_H_

#include <vnl/io.h>
#include <vnl/Basic.h>
#include <vnl/Hash.h>
#include <vnl/Vector.h>
#include <vnl/String.h>
#include <vnl/Binary.h>
#include <vnl/Array.h>
#include <vnl/List.h>
#include <vnl/Map.h>
#include <vnl/Pool.h>


namespace vnl {

class Var;

class Type : public vnl::io::Serializable {
public:
	Type() {}
	
	virtual ~Type() {}
	
	virtual uint32_t get_vni_hash() const = 0;
	virtual const char* get_type_name() const = 0;
	
	virtual int get_type_size() const = 0;
	virtual int get_num_fields() const = 0;
	virtual int get_field_index(vnl::Hash32 hash) const = 0;
	virtual const char* get_field_name(int index) const = 0;
	virtual void get_field(int index, vnl::String& str) const = 0;
	virtual void set_field(int index, const vnl::String& str) = 0;
	virtual void get_field(int index, vnl::io::TypeOutput& out) const = 0;
	virtual void set_field(int index, vnl::io::TypeInput& in) = 0;
	virtual void get_field(int index, vnl::Var& var) const = 0;
	virtual void set_field(int index, const vnl::Var& var) = 0;
	
	virtual void from_string(const vnl::String& str);
	virtual void to_string_ex(vnl::String& str) const;
	
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
