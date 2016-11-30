/*
 * Type.cpp
 *
 *  Created on: Nov 30, 2016
 *      Author: mad
 */

#include <vnl/Type.hxx>
#include <vnl/BinaryValue.h>


namespace vnl {

Value* read(vnl::io::TypeInput& in) {
	Value* obj = 0;
	int size = 0;
	int id = in.getEntry(size);
	switch(id) {
		case VNL_IO_CLASS: {
			uint32_t hash = 0;
			in.getHash(hash);
			obj = vnl::create(hash);
			if(!obj) {
				BinaryValue* bin = BinaryValue::create();
				bin->hash = hash;
				obj = bin;
			}
			obj->deserialize(in, size);
			break;
		}
		default: in.skip(id, size);
	}
	return obj;
}

void read(vnl::io::TypeInput& in, Value& obj) {
	int size = 0;
	int id = in.getEntry(size);
	switch(id) {
		case VNL_IO_STRUCT:
			obj.deserialize(in, size);
			break;
		case VNL_IO_CLASS: {
			uint32_t hash = 0;
			in.getHash(hash);
			obj.deserialize(in, size);
			break;
		}
		default: in.skip(id, size);
	}
}

void read(vnl::io::TypeInput& in, Value* obj) {
	if(obj) {
		read(in, *obj);
	} else {
		in.skip();
	}
}


} // vnl
