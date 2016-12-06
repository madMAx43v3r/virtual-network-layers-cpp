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
#include <vnl/Layer.h>


namespace vnl {

/*
 * Function to create any Value based on the type hash.
 */
Value* create(vnl::Hash32 hash);

/*
 * Returns all class names known to this process.
 */
Array<String> get_class_names();


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

template<typename T>
void read(vnl::io::TypeInput& in, Array<T>& obj) {
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

template<typename T>
void read(vnl::io::TypeInput& in, List<T>& obj) {
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

template<typename K, typename V>
void read(vnl::io::TypeInput& in, Map<K,V>& obj) {
	int size = 0;
	int id = in.getEntry(size);
	if(id == VNL_IO_ARRAY && size % 2 == 0) {
		for(int i = 0; i < size && !in.error(); i += 2) {
			K key;
			V value;
			vnl::read(in, key);
			vnl::read(in, value);
			obj[key] = value;
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

template<typename T>
void write(vnl::io::TypeOutput& out, const Array<T>& obj) {
	out.putEntry(VNL_IO_ARRAY, obj.size());
	for(typename vnl::Array<T>::const_iterator iter = obj.begin();
			iter != obj.end() && !out.error(); ++iter)
	{
		vnl::write(out, *iter);
	}
}

template<typename T>
void write(vnl::io::TypeOutput& out, const List<T>& obj) {
	out.putEntry(VNL_IO_ARRAY, obj.size());
	for(typename vnl::List<T>::const_iterator iter = obj.begin();
			iter != obj.end() && !out.error(); ++iter)
	{
		vnl::write(out, *iter);
	}
}

template<typename K, typename V>
void write(vnl::io::TypeOutput& out, const Map<K,V>& obj) {
	out.putEntry(VNL_IO_ARRAY, obj.size()*2);
	for(typename vnl::Map<K,V>::const_iterator iter = obj.begin();
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
	// TODO: escape
	str << "\"" << obj << "\"";
}

inline void to_string(vnl::String& str, const Binary& obj) {
	// TODO
	str << "\"\"";
}

template<typename T>
void to_string(vnl::String& str, const Array<T>& obj) {
	to_string(str, obj.begin(), obj.end());
}

template<typename T>
void to_string(vnl::String& str, const List<T>& obj) {
	to_string(str, obj.begin(), obj.end());
}

template<typename K, typename V>
void to_string(vnl::String& str, const Map<K,V>& obj) {
	to_string(str, obj.begin(), obj.end());
}

template<typename K, typename V>
void to_string(vnl::String& str, const vnl::pair<K,V>& obj) {
	str << "{\"key\": "; to_string(str, obj.first);
	str << ", \"value\": "; to_string(str, obj.second); str << "}";
}

template<class Iter>
void to_string(vnl::String& str, Iter first, Iter last) {
	str << "[";
	for(Iter it = first; it != last; ++it) {
		if(it != first) {
			str << ", ";
		}
		to_string(str, *it);
	}
	str << "]";
}

template<typename T, int N>
void to_string(vnl::String& str, const vnl::Vector<T, N>& vec) {
	str << "[";
	for(int i = 0; i < vec.size(); ++i) {
		if(i > 0) {
			str << ", ";
		}
		to_string(str, vec[i]);
	}
	str << "]";
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
	obj = str;
}

inline void from_string(const vnl::String& str, Binary& obj) {
	// TODO
	assert(false);
}

template<typename T>
void from_string(const vnl::String& str, Array<T>& obj) {
	int i = 0;
	vnl::String buf;
	for(vnl::String::const_iterator it = str.begin(); it != str.end(); ++it) {
		char c = *it;
		if(c == ']' || c == ',') {
			from_string(buf, obj.push_back());
			buf.clear();
			i++;
		} else if(c != '[') {
			buf.push_back(c);
		}
	}
}

template<typename T>
void from_string(const vnl::String& str, List<T>& obj) {
	int i = 0;
	vnl::String buf;
	for(vnl::String::const_iterator it = str.begin(); it != str.end(); ++it) {
		char c = *it;
		if(c == ']' || c == ',') {
			from_string(buf, *obj.push_back());
			buf.clear();
			i++;
		} else if(c != '[') {
			buf.push_back(c);
		}
	}
}

template<typename K, typename V>
void from_string(const vnl::String& str, Map<K,V>& obj) {
	// TODO
	assert(false);
}

template<typename T, int N>
void from_string(const vnl::String& str, vnl::Vector<T, N>& vec) {
	int i = 0;
	vnl::String buf;
	for(vnl::String::const_iterator it = str.begin(); it != str.end(); ++it) {
		char c = *it;
		if(c == ']' || c == ',') {
			from_string(buf, vec[i]);
			buf.clear();
			i++;
		} else if(c != '[') {
			buf.push_back(c);
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
bool read_config(String domain, String topic, String name, T& ref) {
	const String* value = Layer::get_config(domain, topic, name);
	if(value) {
		vnl::from_string(*value, ref);
		return true;
	}
	return false;
}

inline bool read_from_file(vnl::Value& value, const char* filename) {
	vnl::io::File file = ::fopen(filename, "r");
	if(file.good()) {
		vnl::io::TypeInput in(&file);
		vnl::read(in, value);
		::fclose(file);
		return !in.error();
	}
	return false;
}

inline bool read_from_file(vnl::Value& value, const vnl::String& filename) {
	char buf[1024];
	filename.to_string(buf, sizeof(buf));
	return read_from_file(value, buf);
}

inline bool write_to_file(const vnl::Value& value, const char* filename) {
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

inline bool write_to_file(const vnl::Value& value, const vnl::String& filename) {
	char buf[1024];
	filename.to_string(buf, sizeof(buf));
	return write_to_file(value, buf);
}



} // vnl

#endif /* INCLUDE_VNI_TYPE_HXX_ */
