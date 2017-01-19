/*
 * Type.cpp
 *
 *  Created on: Nov 30, 2016
 *      Author: mad
 */

#include <vnl/Type.hxx>
#include <vnl/Var.h>
#include <vnl/BinaryValue.h>


namespace vnl {

namespace internal {
	
	GlobalPool* global_pool_ = 0;
	Map<String, String>* config_ = 0;
	
} // internal

void Type::from_string(const vnl::String& str) {
	String field;
	String buf;
	String::const_iterator it = str.begin();
	String::const_iterator end = str.end();
	while(it != end) {
		char c = *it;
		if(c == '{' || c == ',') {
			it++;
			internal::parse_value(it, end, buf);
			vnl::from_string(buf, field);
			while(it != end) {
				c = *it;
				it++;
				if(c == ':') {
					break;
				}
			}
			internal::parse_value(it, end, buf);
			set_field(get_field_index(field), buf);
			if(it == end) {
				break;
			}
			continue;
		} else if(c == '}') {
			break;
		}
		it++;
	}
}

void Type::to_string_ex(vnl::String& str) const {
	str.push_back('{');
	int N = get_num_fields();
	for(int i = 0; i < N; ++i) {
		if(i > 0) {
			str << ", ";
		}
		str << "\"" << get_field_name(i) << "\": ";
		get_field(i, str);
	}
	str.push_back('}');
}

void Interface::get_field(int index, vnl::Var& var) const {
	var.clear();
}

void Interface::set_field(int index, const vnl::Var& var) {
	// nothing
}

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

template<>
bool read_config<String>(String domain, String topic, String name, String& ref) {
	const String* value = vnl::get_config(domain, topic, name);
	if(value) {
		ref = *value;
		return true;
	}
	return false;
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


namespace internal {

void parse_value(String::const_iterator& it, const String::const_iterator& end, String& out) {
	out.clear();
	int stack = 0;
	bool string = false;
	bool escape = false;
	bool done = false;
	while(!done && it != end) {
		char c = *it;
		if(string) {
			if(c == '"' && !escape) {
				string = false;
				stack--;
				if(stack == 0) {
					done = true;
				}
			} else if(c == '\\' && !escape) {
				escape = true;
				it++;
				continue;
			}
		} else if(c == '"') {
			string = true;
			stack++;
		} else if(c == ',' && stack == 0) {
			break;
		} else if(c == '}' && stack == 0) {
			break;
		} else if(c == '{' || c == '[') {
			stack++;
		} else if(c == '}' || c == ']') {
			stack--;
			if(stack <= 0) {
				done = true;
			}
		}
		if(stack > 0 || c != ' ') {
			out.push_back(c);
		}
		it++;
	}
}

} // internal

} // vnl
