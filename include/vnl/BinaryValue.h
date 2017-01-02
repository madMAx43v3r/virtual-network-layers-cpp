/*
 * BinaryValue.hxx
 *
 *  Created on: Sep 17, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNI_GENERATED_vnl_BinaryValue_HXX_
#define INCLUDE_VNI_GENERATED_vnl_BinaryValue_HXX_

#include <vnl/Hash32.h>
#include <vnl/Value.hxx>

#include <vnl/Type.h>


namespace vnl {

class BinaryValue : public vnl::Value {
public:
	static const uint32_t VNI_HASH = 0x8d393c45;
	
	vnl::Hash32 hash;
	int32_t size;
	vnl::Binary data;
	
	BinaryValue() {
		size = 0;
	}
	
	static BinaryValue* create();
	virtual BinaryValue* clone() const;
	virtual void destroy();
	
	virtual void serialize(vnl::io::TypeOutput& _out) const;
	virtual void deserialize(vnl::io::TypeInput& _in, int _size);
	
	virtual uint32_t get_vni_hash() const { return VNI_HASH; }
	virtual const char* get_type_name() const { return "vnl.BinaryValue"; }
	
	virtual int type_size() const { return sizeof(BinaryValue); }
	
};

} // namespace

#endif // INCLUDE_VNI_GENERATED_vnl_BinaryValue_HXX_
