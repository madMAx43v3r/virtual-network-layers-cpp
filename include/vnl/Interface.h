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

class Value;
class Packet;

class Interface : public Type {
public:
	virtual ~Interface() {}
	
	virtual void serialize(vnl::io::TypeOutput& out) const {
		out.putNull();
	}
	
	virtual void deserialize(vnl::io::TypeInput& in, int size) {
		int stack = 0;
		while(!in.error()) {
			int size = 0;
			int id = in.getEntry(size);
			if(id == VNL_IO_CALL) {
				uint32_t hash = 0;
				in.getHash(hash);
				if(!vni_call(in, hash, size)) {
					for(int i = 0; i < size; ++i) {
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
	
	virtual int get_num_fields() const {
		return 0;
	}
	
	virtual int get_field_index(vnl::Hash32 hash_) const {
		return -1;
	}
	
	virtual const char* get_field_name(int index_) const {
		return 0;
	}
	
	virtual void get_field(int index_, vnl::String& str_) const {
		// default: nothing
	}
	
	virtual void set_field(int index_, const vnl::String& str_) {
		// default: nothing
	}
	
	virtual void get_field(int _index, vnl::io::TypeOutput& _out) const {
		_out.putNull();
	}
	
	virtual void set_field(int _index, vnl::io::TypeInput& _in) {
		_in.skip();
	}
	
protected:
	virtual bool vni_call(vnl::io::TypeInput& in, uint32_t hash, int num_args) {
		return false;
	}
	
	virtual bool vni_const_call(vnl::io::TypeInput& in, uint32_t hash, int num_args, vnl::io::TypeOutput& out) {
		return false;
	}
	
	virtual bool handle_switch(vnl::Value* sample, vnl::Packet* packet) {
		return false;
	}
	
	virtual bool handle_switch(vnl::Value* sample, vnl::Basic* input) {
		return false;
	}
	
};


} //vnl

#endif /* INCLUDE_VNI_INTERFACE_H_ */
