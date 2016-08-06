/*
 * Interface.h
 *
 *  Created on: Jun 30, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNI_INTERFACE_H_
#define INCLUDE_VNI_INTERFACE_H_

#include <vnl/Type.h>


namespace vnl {

class Interface : public Type {
public:
	virtual ~Interface() {}
	
	virtual void deserialize(vnl::io::TypeInput& in, int size) {
		int stack = 1;
		while(!in.error()) {
			int size = 0;
			int id = in.getEntry(size);
			if(id == VNL_IO_CALL) {
				uint32_t hash = 0;
				in.getHash(hash);
				if(!vni_call(in, hash, size)) {
					for(uint32_t i = 0; i < size; ++i) {
						in.skip();
					}
				}
			} else if(id == VNL_IO_INTERFACE) {
				if(size == VNL_IO_BEGIN) {
					uint32_t hash = 0;
					in.getHash(hash);
					stack++;
				} else if(size == VNL_IO_END) {
					stack--;
					if(stack == 0) {
						break;
					}
				}
			} else {
				in.skip(id, size);
			}
		}
	}
	
	virtual int num_fields() const {
		return 0;
	}
	
	virtual int field_index(vnl::Hash32 hash_) const {
		return -1;
	}
	
	virtual const char* field_name(int index_) const {
		return "?";
	}
	
	virtual void get_field(int index_, vnl::String& str_) const {
		// default: nothing
	}
	
	virtual void set_field(int index_, vnl::io::ByteInput& in_) {
		// default: nothing
	}
	
protected:
	virtual bool vni_call(vnl::io::TypeInput& in, uint32_t hash, int num_args) {
		return false;
	}
	
	virtual bool vni_const_call(vnl::io::TypeInput& in, uint32_t hash, int num_args, vnl::io::TypeOutput& out) {
		return false;
	}
	
};



}

#endif /* INCLUDE_VNI_INTERFACE_H_ */