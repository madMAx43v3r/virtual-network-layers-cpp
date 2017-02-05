/*
 * Type.hxx
 *
 *  Created on: Jul 12, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNI_TYPE_HXX_
#define INCLUDE_VNI_TYPE_HXX_

#include <vnl/Type.h>
#include <vnl/Enum.h>
#include <vnl/Interface.h>
#include <vnl/Value.hxx>


namespace vnl {

namespace internal {
	
	extern GlobalPool* global_pool_;
	extern Map<String, String>* config_;
	
	void parse_value(String::const_iterator& it, const String::const_iterator& end, String& out);
	
} // internal

/*
 * Function to create any Value based on the type hash.
 */
Value* create(vnl::Hash32 hash);

/*
 * Returns all class names known to this process.
 */
Array<String> get_class_names();


/*
 * Memory functions
 */
template<typename T>
T* create() {
	T* obj = internal::global_pool_->create<T>();
	return obj;
}

template<typename T>
T* clone(const T& other) {
	return new(internal::global_pool_->alloc(sizeof(T))) T(other);
}

template<typename T>
void destroy(T* obj) {
	if(obj) {
		obj->destroy();
	}
}


/*
 * Generic reader functions
 */
inline void read(vnl::io::TypeInput& in, bool& val) { in.getValue(val); }
inline void read(vnl::io::TypeInput& in, int8_t& val) { in.getValue(val); }
inline void read(vnl::io::TypeInput& in, int16_t& val) { in.getValue(val); }
inline void read(vnl::io::TypeInput& in, int32_t& val) { in.getValue(val); }
inline void read(vnl::io::TypeInput& in, int64_t& val) { in.getValue(val); }
inline void read(vnl::io::TypeInput& in, float& val) { in.getValue(val); }
inline void read(vnl::io::TypeInput& in, double& val) { in.getValue(val); }

Value* read(vnl::io::TypeInput& in);
void read(vnl::io::TypeInput& in, Value& obj);
void read(vnl::io::TypeInput& in, Value* obj);

inline void read(vnl::io::TypeInput& in, Interface& obj) {
	obj.deserialize(in, 0);
}

inline void read(vnl::io::TypeInput& in, String& obj) {
	in.getString(obj);
}

inline void read(vnl::io::TypeInput& in, Binary& obj) {
	in.getBinary(obj.data, obj.size);
}

template<typename T, typename TPage>
void read(vnl::io::TypeInput& in, Array<T,TPage>& obj) {
	int size = 0;
	int id = in.getEntry(size);
	if(id == VNL_IO_ARRAY) {
		for(int i = 0; i < size && !in.error(); ++i) {
			vnl::read(in, obj.push_back());
		}
	} else {
		in.skip(id, size);
	}
}

template<typename T, typename TPage>
void read(vnl::io::TypeInput& in, List<T,TPage>& obj) {
	int size = 0;
	int id = in.getEntry(size);
	if(id == VNL_IO_ARRAY) {
		for(int i = 0; i < size && !in.error(); ++i) {
			vnl::read(in, *obj.push_back());
		}
	} else {
		in.skip(id, size);
	}
}

template<typename K, typename V, typename TPage>
void read(vnl::io::TypeInput& in, Map<K,V,TPage>& obj) {
	int size = 0;
	int id = in.getEntry(size);
	if(id == VNL_IO_MAP) {
		for(int i = 0; i < size && !in.error(); ++i) {
			K key;
			vnl::read(in, key);
			V& value = obj[key];
			vnl::read(in, value);
		}
	} else {
		in.skip(id, size);
	}
}

template<typename T, int N>
void read(vnl::io::TypeInput& in, vnl::Vector<T, N>& vec) {
	int size = 0;
	int id = in.getEntry(size);
	if(id == VNL_IO_ARRAY) {
		for(int i = 0; i < size && !in.error(); ++i) {
			vnl::read(in, vec[i]);
		}
	} else {
		in.skip(id, size);
	}
}


/*
 * Generic writer functions
 */
inline void write(vnl::io::TypeOutput& out, const bool& val) { out.putValue(val); }
inline void write(vnl::io::TypeOutput& out, const int8_t& val) { out.putValue(val); }
inline void write(vnl::io::TypeOutput& out, const int16_t& val) { out.putValue(val); }
inline void write(vnl::io::TypeOutput& out, const int32_t& val) { out.putValue(val); }
inline void write(vnl::io::TypeOutput& out, const int64_t& val) { out.putValue(val); }
inline void write(vnl::io::TypeOutput& out, const float& val) { out.putValue(val); }
inline void write(vnl::io::TypeOutput& out, const double& val) { out.putValue(val); }

inline void write(vnl::io::TypeOutput& out, const Value& obj) {
	obj.serialize(out);
}

inline void write(vnl::io::TypeOutput& out, const Value* obj) {
	if(obj) {
		write(out, *obj);
	} else {
		out.putNull();
	}
}

inline void write(vnl::io::TypeOutput& out, const Interface& obj) {
	obj.serialize(out);
}

inline void write(vnl::io::TypeOutput& out, const String& obj) {
	out.putString(obj);
}

inline void write(vnl::io::TypeOutput& out, const Binary& obj) {
	out.putBinary(obj.data, obj.size);
}

template<typename T, typename TPage>
void write(vnl::io::TypeOutput& out, const Array<T,TPage>& obj) {
	out.putEntry(VNL_IO_ARRAY, obj.size());
	for(typename vnl::Array<T,TPage>::const_iterator iter = obj.begin();
			iter != obj.end() && !out.error(); ++iter)
	{
		vnl::write(out, *iter);
	}
}

template<typename T, typename TPage>
void write(vnl::io::TypeOutput& out, const List<T,TPage>& obj) {
	out.putEntry(VNL_IO_ARRAY, obj.size());
	for(typename vnl::List<T,TPage>::const_iterator iter = obj.begin();
			iter != obj.end() && !out.error(); ++iter)
	{
		vnl::write(out, *iter);
	}
}

template<typename K, typename V, typename TPage>
void write(vnl::io::TypeOutput& out, const Map<K,V,TPage>& obj) {
	out.putEntry(VNL_IO_MAP, obj.size());
	for(typename vnl::Map<K,V,TPage>::const_iterator iter = obj.begin();
			iter != obj.end() && !out.error(); ++iter)
	{
		vnl::write(out, iter->first);
		vnl::write(out, iter->second);
	}
}

template<typename T, int N>
void write(vnl::io::TypeOutput& out, const vnl::Vector<T, N>& vec) {
	out.putEntry(VNL_IO_ARRAY, vec.size());
	for(int i = 0; i < vec.size(); ++i) {
		vnl::write(out, vec[i]);
	}
}


/*
 * Generic to_string functions
 */
inline void to_string(vnl::String& str, const Value& obj) {
	return obj.to_string_ex(str);
}

inline void to_string(vnl::String& str, const Value* obj) {
	if(obj) {
		obj->to_string_ex(str);
	} else {
		str << "{}";
	}
}

inline void to_string(vnl::String& str, const Interface& obj) {
	obj.to_string_ex(str);
}

inline void to_string(vnl::String& str, const String& obj) {
	str.push_back('"');
	for(String::const_iterator it = obj.begin(); it != obj.end(); ++it) {
		char c = *it;
		if(c == '"') {
			str.push_back('\\');
		}
		str.push_back(c);
	}
	str.push_back('"');
}

inline void to_string(vnl::String& str, const Binary& obj) {
	str << "\"\"";
}

template<class Iter>
void to_string(vnl::String& str, Iter first, Iter last) {
	str.push_back('[');
	for(Iter it = first; it != last; ++it) {
		if(it != first) {
			str << ", ";
		}
		to_string(str, *it);
	}
	str.push_back(']');
}

template<typename T, typename TPage>
void to_string(vnl::String& str, const Array<T,TPage>& obj) {
	to_string(str, obj.begin(), obj.end());
}

template<typename T, typename TPage>
void to_string(vnl::String& str, const List<T,TPage>& obj) {
	to_string(str, obj.begin(), obj.end());
}

template<typename K, typename V, typename TPage>
void to_string(vnl::String& str, const Map<K,V,TPage>& obj) {
	to_string(str, obj.begin(), obj.end());
}

template<typename K, typename V>
void to_string(vnl::String& str, const vnl::pair<K,V>& obj) {
	str.push_back('['); to_string(str, obj.first);
	str << ", "; to_string(str, obj.second); str.push_back(']');
}

template<typename T, int N>
void to_string(vnl::String& str, const vnl::Vector<T,N>& vec) {
	str.push_back('[');
	for(int i = 0; i < vec.size(); ++i) {
		if(i > 0) {
			str << ", ";
		}
		to_string(str, vec[i]);
	}
	str.push_back(']');
}

inline void to_string(vnl::String& str, const bool& val) { str << (val ? "true" : "false"); }
inline void to_string(vnl::String& str, const int8_t& val) { str << vnl::dec(val); }
inline void to_string(vnl::String& str, const int16_t& val) { str << vnl::dec(val); }
inline void to_string(vnl::String& str, const int32_t& val) { str << vnl::dec(val); }
inline void to_string(vnl::String& str, const int64_t& val) { str << vnl::dec(val); }
inline void to_string(vnl::String& str, const float& val) { str << vnl::def(val, 6); }
inline void to_string(vnl::String& str, const double& val) { str << vnl::def(val, 12); }

template<typename T>
vnl::String to_string(const T& ref) {
	vnl::String str;
	to_string(str, ref);
	return str;
}


/*
 * Generic from_string functions
 */
inline void from_string(const vnl::String& str, Value& obj) {
	obj.from_string(str);
}

inline void from_string(const vnl::String& str, Value* obj) {
	if(obj) {
		obj->from_string(str);
	}
}

inline void from_string(const vnl::String& str, Interface& obj) {
	obj.from_string(str);
}

inline void from_string(const vnl::String& str, String& obj) {
	obj.clear();
	int stack = 0;
	bool escape = false;
	for(String::const_iterator it = str.begin(); it != str.end(); ++it) {
		char c = *it;
		if(c == '\\' && !escape) {
			escape = true;
		} else if(c == '"' && !escape) {
			if(stack == 0) {
				stack = 1;
			} else {
				break;
			}
		} else {
			obj.push_back(c);
			escape = false;
		}
	}
}

inline void from_string(const vnl::String& str, Binary& obj) {
	// nothing
}

template<typename T, typename TPage>
void from_string(const vnl::String& str, Array<T,TPage>& obj) {
	obj.clear();
	String buf;
	int stack = 0;
	for(String::const_iterator it = str.begin(); it != str.end(); ++it) {
		char c = *it;
		if(c == '[') {
			stack++;
		} else if(c == ']') {
			break;
		} else if(stack && c != ',') {
			internal::parse_value(it, str.end(), buf);
			from_string(buf, obj.push_back());
			if(it == str.end()) {
				break;
			}
		}
	}
}

template<typename T, typename TPage>
void from_string(const vnl::String& str, List<T,TPage>& obj) {
	obj.clear();
	String buf;
	int stack = 0;
	for(String::const_iterator it = str.begin(); it != str.end(); ++it) {
		char c = *it;
		if(c == '[') {
			stack++;
		} else if(c == ']') {
			break;
		} else if(stack && c != ',') {
			internal::parse_value(it, str.end(), buf);
			from_string(buf, *obj.push_back());
			if(it == str.end()) {
				break;
			}
		}
	}
}

template<typename K, typename V>
void from_string(const vnl::String& str, vnl::pair<K,V>& obj) {
	String buf;
	int stack = 0;
	int state = 0;
	for(String::const_iterator it = str.begin(); it != str.end(); ++it) {
		char c = *it;
		if(c == '[') {
			stack++;
		} else if(c == ']') {
			break;
		} else if(c == ',') {
			stack++;
		} else if(stack > state) {
			internal::parse_value(it, str.end(), buf);
			if(stack == 1) {
				from_string(buf, obj.first);
				state++;
			} else {
				from_string(buf, obj.second);
			}
			if(it == str.end()) {
				break;
			}
		}
	}
}

template<typename K, typename V, typename TPage>
void from_string(const vnl::String& str, Map<K,V,TPage>& obj) {
	obj.clear();
	String buf;
	int stack = 0;
	for(String::const_iterator it = str.begin(); it != str.end(); ++it) {
		char c = *it;
		if(c == '[') {
			stack++;
		} else if(c == ']') {
			break;
		} else if(stack && c != ',') {
			internal::parse_value(it, str.end(), buf);
			vnl::pair<K,V> entry;
			from_string(buf, entry);
			obj[entry.first] = entry.second;
			if(it == str.end()) {
				break;
			}
		}
	}
}

template<typename T, int N>
void from_string(const vnl::String& str, vnl::Vector<T,N>& vec) {
	int i = 0;
	String buf;
	int stack = 0;
	for(String::const_iterator it = str.begin(); it != str.end(); ++it) {
		char c = *it;
		if(c == '[') {
			stack++;
		} else if(c == ']') {
			break;
		} else if(stack && c != ',') {
			internal::parse_value(it, str.end(), buf);
			from_string(buf, vec[i]);
			i++;
			if(it == str.end()) {
				break;
			}
		}
	}
}

inline int64_t atoi(const vnl::String& str) {
	char buf[256];
	str.to_string(buf, sizeof(buf));
	return ::atoi(buf);
}

inline int64_t atol(const vnl::String& str) {
	char buf[256];
	str.to_string(buf, sizeof(buf));
	return ::atol(buf);
}

inline double atof(const vnl::String& str) {
	char buf[256];
	str.to_string(buf, sizeof(buf));
	return ::atof(buf);
}

inline void from_string(const vnl::String& str, bool& val) { val = str == "true"; }
inline void from_string(const vnl::String& str, int8_t& val) { val = atoi(str); }
inline void from_string(const vnl::String& str, int16_t& val) { val = atoi(str); }
inline void from_string(const vnl::String& str, int32_t& val) { val = atoi(str); }
inline void from_string(const vnl::String& str, int64_t& val) { val = atol(str); }
inline void from_string(const vnl::String& str, float& val) { val = atof(str); }
inline void from_string(const vnl::String& str, double& val) { val = atof(str); }


/*
 * Generic util functions
 */
template<typename T>
Binary to_binary(const T& value) {
	Binary bin;
	bin.data = Page::alloc();
	vnl::io::ByteBuffer buf(bin.data);
	vnl::io::TypeOutput out(&buf);
	vnl::write(out, value);
	out.flush();
	bin.size = buf.position();
	return bin;
}

inline Binary to_binary(const char* str) {
	return to_binary<vnl::String>(str);
}

const String* get_config(const String& domain, const String& topic, const String& name);

template<typename T>
bool read_config(String domain, String topic, String name, T& ref) {
	const String* value = vnl::get_config(domain, topic, name);
	if(value) {
		vnl::from_string(*value, ref);
		return true;
	}
	return false;
}

template<>
bool read_config<String>(String domain, String topic, String name, String& ref);

bool read_from_file(vnl::Value& value, const char* filename);
bool read_from_file(vnl::Value& value, const vnl::String& filename);

bool write_to_file(const vnl::Value& value, const char* filename);
bool write_to_file(const vnl::Value& value, const vnl::String& filename);


} // vnl

#endif /* INCLUDE_VNI_TYPE_HXX_ */
