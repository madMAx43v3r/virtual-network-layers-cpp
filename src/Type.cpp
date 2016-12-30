/*
 * Type.cpp
 *
 *  Created on: Nov 30, 2016
 *      Author: mad
 */

#include <vnl/Type.hxx>
#include <vnl/BinaryValue.h>


namespace vnl {

namespace internal {
	
	GlobalPool* global_pool_ = 0;
	Map<String, String>* config_ = 0;
	
} // internal


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

const String* get_config(const String& domain, const String& topic, const String& name) {
	static std::mutex mutex;
	String key;
	key << domain << ":" << topic << "->" << name;
	std::lock_guard<std::mutex> lock(mutex);
	return internal::config_->find(key);
}

bool read_from_file(vnl::Value& value, const char* filename) {
	vnl::io::File file = ::fopen(filename, "r");
	if(file.good()) {
		vnl::io::TypeInput in(&file);
		vnl::read(in, value);
		::fclose(file);
		return !in.error();
	}
	return false;
}

bool read_from_file(vnl::Value& value, const vnl::String& filename) {
	char buf[1024];
	filename.to_string(buf, sizeof(buf));
	return read_from_file(value, buf);
}

bool write_to_file(const vnl::Value& value, const char* filename) {
	vnl::io::File file = ::fopen(filename, "w");
	if(file.good()) {
		vnl::io::TypeOutput out(&file);
		vnl::write(out, value);
		out.flush();
		::fclose(file);
		return !out.error();
	}
	return false;
}

bool write_to_file(const vnl::Value& value, const vnl::String& filename) {
	char buf[1024];
	filename.to_string(buf, sizeof(buf));
	return write_to_file(value, buf);
}


} // vnl
