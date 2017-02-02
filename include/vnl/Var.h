/*
 * Var.h
 *
 *  Created on: Jan 5, 2017
 *      Author: mad
 */

#ifndef INCLUDE_VNL_VAR_H_
#define INCLUDE_VNL_VAR_H_

#include <vnl/String.h>
#include <vnl/Pointer.h>
#include <vnl/Enum.h>
#include <vnl/Array.h>
#include <vnl/List.h>
#include <vnl/Map.h>

#include <vnl/Value.hxx>
#include <vnl/IOException.hxx>
#include <vnl/MemoryException.hxx>


namespace vnl {

class VarIterator;

class Var {
public:
	enum {
		NIL, BOOL, INT, REAL, STRING, VALUE, LIST, MAP, PVAR, CPVAR
	};
	
	Var() : type(NIL) { mem_ = 0; }
	Var(const bool& v) : type(BOOL) { bool_ = v; }
	Var(const int8_t& v) : type(INT) { long_ = v; }
	Var(const int16_t& v) : type(INT) { long_ = v; }
	Var(const int32_t& v) : type(INT) { long_ = v; }
	Var(const int64_t& v) : type(INT) { long_ = v; }
	Var(const uint8_t& v) : type(INT) { mem_ = v; }
	Var(const uint16_t& v) : type(INT) { mem_ = v; }
	Var(const uint32_t& v) : type(INT) { mem_ = v; }
	Var(const uint64_t& v) : type(INT) { mem_ = v; }
	Var(const float& v) : type(REAL) { double_ = v; }
	Var(const double& v) : type(REAL) { double_ = v; }
	Var(const char* v) : type(NIL) { *this = String(v); }
	Var(const String& v) : type(NIL) { *this = v; }
	Var(const Value& v) : type(NIL) { *this = v; }
	Var(const Hash32& v) : type(NIL) { *this = v; }
	Var(const Hash64& v) : type(NIL) { *this = v; }
	Var(const Var* v) : type(CPVAR) { cvar_ = v; if(var_) { var_->ref_count++; } }
	Var(Var* v) : type(PVAR) { var_ = v; if(var_) { var_->ref_count++; } }
	
	template<typename T>
	Var(const List<T>& v) : type(NIL) { *this = v; }
	
	template<typename T>
	Var(const Array<T>& v) : type(NIL) { *this = v; }
	
	template<typename K, typename V>
	Var(const Map<K,V>& v) : type(NIL) { *this = v; }
	
	template<typename T>
	Var(const T& v) : type(NIL) {}
	
	Var(const Var& v) : type(NIL) {
		*this = v;
	}
	
	~Var() {
		if(ref_count) {
			throw MemoryException();
		}
		clear();
	}
	
	Var& operator=(bool v) {
		clear();
		type = BOOL;
		bool_ = v;
		return *this;
	}
	
	Var& operator=(int8_t v) {
		clear();
		type = INT;
		long_ = v;
		return *this;
	}
	
	Var& operator=(int16_t v) {
		clear();
		type = INT;
		long_ = v;
		return *this;
	}
	
	Var& operator=(int32_t v) {
		clear();
		type = INT;
		long_ = v;
		return *this;
	}
	
	Var& operator=(int64_t v) {
		clear();
		type = INT;
		long_ = v;
		return *this;
	}
	
	Var& operator=(uint8_t v) {
		clear();
		type = INT;
		mem_ = v;
		return *this;
	}
	
	Var& operator=(uint16_t v) {
		clear();
		type = INT;
		mem_ = v;
		return *this;
	}
	
	Var& operator=(uint32_t v) {
		clear();
		type = INT;
		mem_ = v;
		return *this;
	}
	
	Var& operator=(uint64_t v) {
		clear();
		type = INT;
		mem_ = v;
		return *this;
	}
	
	Var& operator=(float v) {
		clear();
		type = REAL;
		double_ = v;
		return *this;
	}
	
	Var& operator=(double v) {
		clear();
		type = REAL;
		double_ = v;
		return *this;
	}
	
	Var& operator=(const char* v) {
		clear();
		type = STRING;
		if(v) {
			string_ = new String(v);
		}
		return *this;
	}
	
	Var& operator=(const String& v) {
		clear();
		type = STRING;
		if(v.size()) {
			string_ = new String(v);
		}
		return *this;
	}
	
	Var& operator=(const Value& v) {
		clear();
		type = VALUE;
		value_ = v.clone();
		return *this;
	}
	
	Var& operator=(const Hash32& v) {
		clear();
		type = INT;
		mem_ = v.value;
		return *this;
	}
	
	Var& operator=(const Hash64& v) {
		clear();
		type = INT;
		mem_ = v.value;
		return *this;
	}
	
	template<typename T>
	Var& operator=(const List<T>& v) {
		clear();
		type = LIST;
		if(v.size()) {
			list_ = new List<Var>();
			for(const T& elem : v) {
				list_->push_back(elem);
			}
		}
		return *this;
	}
	
	template<typename T>
	Var& operator=(const Array<T>& v) {
		clear();
		type = LIST;
		if(v.size()) {
			list_ = new List<Var>();
			for(const T& elem : v) {
				list_->push_back(elem);
			}
		}
		return *this;
	}
	
	template<typename K, typename V>
	Var& operator=(const Map<K,V>& v) {
		clear();
		type = MAP;
		if(v.size()) {
			map_ = new Map<Var,Var>();
			for(const pair<K,V>& elem : v) {
				(*map_)[elem.first] = elem.second;
			}
		}
		return *this;
	}
	
	Var& operator=(Var* v) {
		clear();
		type = PVAR;
		var_ = v;
		if(var_) { var_->ref_count++; }
		return *this;
	}
	
	Var& operator=(const Var* v) {
		clear();
		type = CPVAR;
		cvar_ = v;
		if(cvar_) { var_->ref_count++; }
		return *this;
	}
	
	Var& operator=(const Var& v) {
		clear();
		switch(v.type) {
			case BOOL: 		bool_ = v.bool_; break;
			case STRING: 	if(v.string_) { string_ = new String(*v.string_); } break;
			case VALUE:		if(v.value_) { value_ = v.value_->clone(); } break;
			case LIST: 		if(v.list_) { list_ = new List<Var>(*v.list_); } break;
			case MAP: 		if(v.map_) { map_ = new Map<Var,Var>(*v.map_); } break;
			case PVAR: 		if(v.var_) { var_ = v.var_; v.var_->ref_count++; } break;
			case CPVAR: 	if(v.cvar_) { cvar_ = v.cvar_; v.var_->ref_count++; } break;
			default: 		mem_ = v.mem_;
		}
		type = v.type;
		return *this;
	}
	
	template<typename T>
	Var& operator=(const T& v) {
		clear();
		return *this;
	}
	
	bool operator==(const Var& v) const {
		if(type == v.type) {
			switch(type) {
				case NIL: 		return true;
				case BOOL: 		return bool_ == v.bool_;
				case STRING:
					if(string_ && v.string_) {
						return *string_ == *v.string_;
					} else if(string_) {
						return !string_->size();
					} else if(v.string_) {
						return !v.string_->size();
					}
					return string_ == v.string_;
				case PVAR:
				case CPVAR:
					if(cvar_ && v.cvar_) {
						return *cvar_ == *v.cvar_;
					}
					return cvar_ == v.cvar_;
			}
			return mem_ == v.mem_;
		}
		switch(type) {
		case BOOL:
			switch(v.type) {
				case INT: 		return bool_ == bool(v.long_);
				case REAL: 		return bool_ == bool(v.double_);
				case STRING: 	return bool_ == bool(v.string_ && v.string_->size());
			}
			return false;
		case INT:
			switch(v.type) {
				case BOOL: 		return bool(long_) == v.bool_;
				case REAL: 		return long_ == int64_t(v.double_);
			}
			return false;
		case REAL:
			switch(v.type) {
				case BOOL: 		return bool(double_) == v.bool_;
				case INT: 		return int64_t(double_) == v.long_;
			}
			return false;
		case PVAR:
		case CPVAR:
			if(cvar_) {
				return *cvar_ == v;
			} else {
				return v.type == NIL;
			}
		}
		return false;
	}
	
	bool operator!=(const Var& v) const {
		return !(*this == v);
	}
	
	bool operator<(const Var& v) const {
		switch(type) {
		case INT:
			switch(v.type) {
				case INT:	return long_ < v.long_;
				case REAL:	return long_ < v.double_;
			}
			break;
		case REAL:
			switch(v.type) {
				case INT:	return double_ < v.long_;
				case REAL:	return double_ < v.double_;
			}
			break;
		case STRING:
			if(v.type == STRING) {
				if(string_ && v.string_) {
					return *string_ < *v.string_;
				} else if(v.string_) {
					return true;
				}
			}
		}
		return false;
	}
	
	bool operator<=(const Var& v) const {
		switch(type) {
		case INT:
			switch(v.type) {
				case INT:	return long_ <= v.long_;
				case REAL:	return long_ <= v.double_;
			}
			break;
		case REAL:
			switch(v.type) {
				case INT:	return double_ <= v.long_;
				case REAL:	return double_ <= v.double_;
			}
			break;
		case STRING:
			if(v.type == STRING) {
				if(string_ && v.string_) {
					return *string_ <= *v.string_;
				} else if(v.string_) {
					return true;
				} else if(!string_) {
					return true;
				}
			}
		}
		return false;
	}
	
	bool operator>(const Var& v) const {
		return !(*this <= v);
	}
	
	bool operator>=(const Var& v) const {
		return !(*this < v);
	}
	
	Var operator+(const Var& v) const {
		switch(type) {
		case INT:
			switch(v.type) {
				case INT:	return long_ + v.long_;
				case REAL:	return long_ + v.double_;
			}
			break;
		case REAL:
			switch(v.type) {
				case INT:	return double_ + v.long_;
				case REAL:	return double_ + v.double_;
			}
		}
		return Var();
	}
	
	Var& operator+=(const Var& v) {
		switch(type) {
		case INT:
			switch(v.type) {
				case INT:	long_ += v.long_; break;
				case REAL:	double_ = long_ + v.double_; type = REAL; break;
			}
			break;
		case REAL:
			switch(v.type) {
				case INT:	double_ += v.long_; break;
				case REAL:	double_ += v.double_; break;
			}
		}
		return *this;
	}
	
	Var operator-(const Var& v) const {
		switch(type) {
		case INT:
			switch(v.type) {
				case INT:	return long_ - v.long_;
				case REAL:	return long_ - v.double_;
			}
			break;
		case REAL:
			switch(v.type) {
				case INT:	return double_ - v.long_;
				case REAL:	return double_ - v.double_;
			}
		}
		return Var();
	}
	
	Var& operator-=(const Var& v) {
		switch(type) {
		case INT:
			switch(v.type) {
				case INT:	long_ -= v.long_; break;
				case REAL:	double_ = long_ - v.double_; type = REAL; break;
			}
			break;
		case REAL:
			switch(v.type) {
				case INT:	double_ -= v.long_; break;
				case REAL:	double_ -= v.double_; break;
			}
		}
		return *this;
	}
	
	Var operator*(const Var& v) const {
		switch(type) {
		case INT:
			switch(v.type) {
				case INT:	return long_ * v.long_;
				case REAL:	return long_ * v.double_;
			}
			break;
		case REAL:
			switch(v.type) {
				case INT:	return double_ * v.long_;
				case REAL:	return double_ * v.double_;
			}
		}
		return Var();
	}
	
	Var& operator*=(const Var& v) {
		switch(type) {
		case INT:
			switch(v.type) {
				case INT:	long_ *= v.long_; break;
				case REAL:	double_ = long_ * v.double_; type = REAL; break;
			}
			break;
		case REAL:
			switch(v.type) {
				case INT:	double_ *= v.long_; break;
				case REAL:	double_ *= v.double_; break;
			}
		}
		return *this;
	}
	
	Var operator/(const Var& v) const {
		switch(type) {
		case INT:
			switch(v.type) {
				case INT:	if(v.long_) { return long_ / v.long_; } break; 
				case REAL:	return double(long_) / v.double_;
			}
			break;
		case REAL:
			switch(v.type) {
				case INT:	return double_ / double(v.long_);
				case REAL:	return double_ / v.double_;
			}
		}
		return Var();
	}
	
	Var& operator/=(const Var& v) {
		switch(type) {
		case INT:
			switch(v.type) {
				case INT:	if(v.long_) { long_ /= v.long_; } else { clear(); } break;
				case REAL:	double_ = double(long_) / v.double_; type = REAL; break;
			}
			break;
		case REAL:
			switch(v.type) {
				case INT:	double_ /= double(v.long_); break;
				case REAL:	double_ /= v.double_; break;
			}
		}
		return *this;
	}
	
	Var& operator<<(const Var& v) {
		return push_back(v);
	}
	
	Var& operator>>(Var& v) {
		v = pop_front();
		return *this;
	}
	
	Var& operator[](const Var& v) {
		if(type != MAP || !map_) {
			clear();
			map_ = new Map<Var,Var>();
			type = MAP;
		}
		return (*map_)[v];
	}
	
	Var get_field(const Hash32& name) {
		Var res;
		if(type == VALUE && value_) {
			int index = value_->get_field_index(name);
			if(index >= 0) {
				value_->get_field(index, res);
			}
		}
		return res;
	}
	
	Var& set_field(const Hash32& name, const Var& v) {
		if(type == VALUE && value_) {
			int index = value_->get_field_index(name);
			if(index >= 0) {
				value_->set_field(index, v);
			}
		}
		return *this;
	}
	
	Var& push_back(const Var& v) {
		if(type == STRING) {
			if(!string_) {
				string_ = new String();
			}
			switch(v.type) {
				case BOOL: 		(*string_) << v.bool_; break;
				case INT: 		(*string_) << v.long_; break;
				case REAL: 		(*string_) << v.double_; break;
				case STRING: 	if(v.string_) { (*string_) << (*v.string_); } break;
				case VALUE:		if(v.value_) { vnl::to_string(*string_, v.value_); } break;
				case LIST: 		if(v.list_) { vnl::to_string(*string_, v.list_); } break;
				case MAP: 		if(v.map_) { vnl::to_string(*string_, v.map_); } break;
				case PVAR: 		if(v.var_) { *this << (*v.var_); } break;
				case CPVAR:		throw IOException();
			}
		} else {
			if(type != LIST || !list_) {
				clear();
				list_ = new List<Var>();
				type = LIST;
			}
			list_->push_back(v);
		}
		return *this;
	}
	
	Var& push_front(const Var& v) {
		if(type == STRING) {
			*this = Var(v).push_back(*this);
		} else {
			if(type != LIST || !list_) {
				clear();
				list_ = new List<Var>();
				type = LIST;
			}
			list_->push_front(v);
		}
		return *this;
	}
	
	Var pop_back() {
		if(type == LIST && list_) {
			return list_->pop_back();
		}
		return Var();
	}
	
	Var pop_front() {
		if(type == LIST && list_) {
			return list_->pop_front();
		}
		return Var();
	}
	
	Var& set(const Var& key, const Var& v) {
		if(type != MAP || !map_) {
			clear();
			map_ = new Map<Var,Var>();
			type = MAP;
		}
		(*map_)[key] = v;
		return *this;
	}
	
	Var get(const Var& key) {
		if(type == MAP && map_) {
			return (*map_)[key];
		}
		return Var();
	}
	
	void erase(const Var& key) {
		if(type == MAP && map_) {
			map_->erase(key);
		}
	}
	
	void to(bool& v) const {
		switch(type) {
			case BOOL: 		v = bool_; break;
			case INT: 		v = long_; break;
			case REAL: 		v = double_; break;
			case STRING: 	if(string_) { v = string_->size(); } else { v = false; } break;
			case LIST: 		if(list_) { v = list_->size(); } else { v = false; } break;
			case MAP: 		if(map_) { v = map_->size(); } else { v = false; } break;
			case CPVAR:
			case PVAR: 		if(cvar_) { cvar_->to(v); } else { v = false; } break;
			default: 		v = false;
		}
	}
	
	void to(int8_t& v) const { to_scalar(v); }
	void to(int16_t& v) const { to_scalar(v); }
	void to(int32_t& v) const { to_scalar(v); }
	void to(int64_t& v) const { to_scalar(v); }
	void to(uint8_t& v) const { to_scalar(v); }
	void to(uint16_t& v) const { to_scalar(v); }
	void to(uint32_t& v) const { to_scalar(v); }
	void to(uint64_t& v) const { to_scalar(v); }
	void to(float& v) const { to_scalar(v); }
	void to(double& v) const { to_scalar(v); }
	
	void to(Hash32& v) const {
		switch(type) {
			case INT: 		v = mem_; break;
			case STRING: 	if(string_) { v = Hash32(*string_); } else { v = 0; } break;
			case CPVAR:
			case PVAR: 		if(cvar_) { cvar_->to(v); } else { v = 0; } break;
			default: 		v = 0;
		}
	}
	
	void to(Hash64& v) const {
		switch(type) {
			case INT: 		v = mem_; break;
			case STRING: 	if(string_) { v = Hash64(*string_); } else { v = 0; } break;
			case CPVAR:
			case PVAR: 		if(cvar_) { cvar_->to(v); } else { v = 0; } break;
			default: 		v = 0;
		}
	}
	
	void to(String& v) const {
		v.clear();
		switch(type) {
			case BOOL: 		vnl::to_string(v, bool_); break;
			case INT: 		vnl::to_string(v, long_); break;
			case REAL: 		vnl::to_string(v, double_); break;
			case STRING: 	if(string_) { v = *string_; } break;
			case VALUE:		if(value_) { vnl::to_string(v, *value_); } else { v = "{}"; } break;
			case LIST: 		if(list_) { vnl::to_string(v, *list_); } else { v = "[]"; } break;
			case MAP: 		if(map_) { vnl::to_string(v, *map_); } else { v = "{}"; } break;
			case CPVAR:
			case PVAR: 		if(cvar_) { cvar_->to(v); } break;
		}
	}
	
	void to(Value& v) const {
		if(type == VALUE && value_) {
			v.assign(*value_);
		}
	}
	
	void to(Enum& v) const {
		// TODO
	}
	
	void to(Var& v) const {
		v = *this;
	}
	
	template<typename T>
	void to(List<T>& v) const {
		v.clear();
		if(type == LIST && list_) {
			for(const Var& elem : *list_) {
				T& ref = *v.push_back();
				elem.to(ref);
			}
		}
	}
	
	template<typename T>
	void to(Array<T>& v) const {
		v.clear();
		if(type == LIST && list_) {
			for(const Var& elem : *list_) {
				T& ref = v.push_back();
				elem.to(ref);
			}
		}
	}
	
	template<typename T, int N>
	void to(Vector<T, N>& v) const {
		for(int i = 0; i < N; ++i) {
			v[i] = T();
		}
		if(type == LIST && list_) {
			int i = 0;
			for(const Var& elem : *list_) {
				elem.to(v[i]);
				i++;
			}
		}
	}
	
	template<typename K, typename V>
	void to(Map<K,V>& v) const {
		v.clear();
		if(type == MAP && map_) {
			for(const pair<Var,Var>& elem : *map_) {
				K key;
				elem.first.to(key);
				V& ref = v[key];
				elem.second.to(ref);
			}
		}
	}
	
	template<typename T>
	void to(Pointer<T>& v) const {
		v.destroy();
		if(type == VALUE && dynamic_cast<T*>(value_)) {
			v = (T*)value_->clone();
		}
	}
	
	template<typename T>
	void to(T& v) const {
		v = T();
	}
	
	template<typename T>
	void to_scalar(T& v) const {
		switch(type) {
			case BOOL: 		v = bool_; break;
			case INT: 		v = long_; break;
			case REAL: 		v = double_; break;
			case CPVAR:
			case PVAR: 		if(cvar_) { cvar_->to_scalar<T>(v); } else { v = T(); } break;
			default: 		v = T();
		}
	}
	
	operator bool() const { bool v; to(v); return v; }
	operator int8_t() const { int8_t v; to(v); return v; }
	operator int16_t() const { int16_t v; to(v); return v; }
	operator int32_t() const { int32_t v; to(v); return v; }
	operator int64_t() const { int64_t v; to(v); return v; }
	operator uint8_t() const { uint8_t v; to(v); return v; }
	operator uint16_t() const { uint16_t v; to(v); return v; }
	operator uint32_t() const { uint32_t v; to(v); return v; }
	operator uint64_t() const { uint64_t v; to(v); return v; }
	operator float() const { float v; to(v); return v; }
	operator double() const { double v; to(v); return v; }
	operator Hash32() const { Hash32 v; to(v); return v; }
	operator Hash64() const { Hash64 v; to(v); return v; }
	
	void clear() {
		switch(type) {
			case STRING: 	delete string_; break;
			case VALUE: 	vnl::destroy(value_); break;
			case LIST: 		delete list_; break;
			case MAP: 		delete map_; break;
			case CPVAR:
			case PVAR: 		if(var_) { var_->ref_count--; } break;
		}
		mem_ = 0;
		type = NIL;
	}
	
	uint64_t hash() const {
		switch(type) {
			case Var::BOOL: 	return vnl::hash(bool_);
			case Var::INT: 		return vnl::hash(long_);
			case Var::REAL: 	return vnl::hash(double_);
			case Var::STRING: 	if(string_) { return vnl::hash(string_); } break;
			case Var::CPVAR:
			case Var::PVAR: 	if(cvar_) { return vnl::hash(*cvar_); } break;
		}
		return 0;
	}
	
	void read(vnl::io::TypeInput& in) {
		clear();
		uint32_t hash = 0;
		int size = 0;
		int id = in.getEntry(size);
		switch(id) {
			case VNL_IO_NULL: break;
			case VNL_IO_BOOL:
				*this = Var(size == VNL_IO_TRUE);
				break;
			case VNL_IO_INTEGER:
				type = Var::INT;
				in.readValue(long_, id, size);
				break;
			case VNL_IO_REAL:
				type = Var::REAL;
				in.readValue(double_, id, size);
				break;
			case VNL_IO_STRING:
				type = Var::STRING;
				string_ = new String();
				in.readString(*string_, size);
				break;
			case VNL_IO_ARRAY:
				type = Var::LIST;
				list_ = new List<Var>();
				for(int i = 0; i < size && !in.error(); ++i) {
					Var& ref = *list_->push_back();
					vnl::read(in, ref);
				}
				break;
			case VNL_IO_MAP:
				type = Var::MAP;
				map_ = new Map<Var,Var>();
				for(int i = 0; i < size && !in.error(); ++i) {
					Var key;
					vnl::read(in, key);
					Var& val = (*map_)[key];
					vnl::read(in, val);
				}
				break;
			case VNL_IO_CLASS:
				in.getHash(hash);
				type = Var::VALUE;
				value_ = vnl::create(hash);
				vnl::read(in, value_);
				break;
			default: in.skip(id, size);
		}
	}
	
	void write(vnl::io::TypeOutput& out) const {
		switch(type) {
			case Var::BOOL: 	vnl::write(out, bool_); break;
			case Var::INT: 		vnl::write(out, long_); break;
			case Var::REAL: 	vnl::write(out, double_); break;
			case Var::STRING: 	if(string_) { vnl::write(out, *string_); } else { out.putNull(); } break;
			case Var::VALUE: 	if(value_) { vnl::write(out, *value_); } else { out.putNull(); } break;
			case Var::LIST: 	if(list_) { vnl::write(out, *list_); } else { out.putNull(); } break;
			case Var::MAP: 		if(map_) { vnl::write(out, *map_); } else { out.putNull(); } break;
			case Var::CPVAR:
			case Var::PVAR: 	if(cvar_) { vnl::write(out, *cvar_); } else { out.putNull(); } break;
			default: 			out.putNull();
		}
	}
	
	void from_string(const vnl::String& str) {
		*this = str;
	}
	
	void to_string(vnl::String& out) const {
		switch(type) {
			case Var::BOOL: 	vnl::to_string(out, bool_); break;
			case Var::INT: 		vnl::to_string(out, long_); break;
			case Var::REAL: 	vnl::to_string(out, double_); break;
			case Var::STRING: 	if(string_) { vnl::to_string(out, *string_); } else { out << "\"\""; } break;
			case Var::VALUE:	if(value_) { vnl::to_string(out, *value_); } else { out << "{}"; } break;
			case Var::LIST: 	if(list_) { vnl::to_string(out, *list_); } else { out << "[]"; } break;
			case Var::MAP: 		if(map_) { vnl::to_string(out, *map_); } else { out << "{}"; } break;
			case Var::CPVAR:
			case Var::PVAR: 	if(cvar_) { vnl::to_string(out, *cvar_); } else { out << "{}"; } break;
			default: 			out << "{}";
		}
	}
	
	String to_string() const {
		String str;
		to_string(str);
		return str;
	}
	
protected:
	union {
		uint64_t mem_;
		bool bool_;
		int64_t long_;
		double double_;
		String* string_;
		Value* value_;
		List<Var>* list_;
		Map<Var,Var>* map_;
		VarIterator* iter_;
		const Var* cvar_;
		Var* var_;
	};
	
	int type;
	int ref_count = 0;
	
};


class VarIterator {
public:
	int index;
	Var key;
	Var it;
	
};


inline uint64_t hash(const Var& obj) {
	return obj.hash();
}

inline void read(vnl::io::TypeInput& in, vnl::Var& obj) {
	obj.read(in);
}

inline void write(vnl::io::TypeOutput& out, const vnl::Var& obj) {
	obj.write(out);
}

inline void from_string(const vnl::String& str, vnl::Var& obj) {
	obj.from_string(str);
}

inline void to_string(vnl::String& out, const vnl::Var& obj) {
	obj.to_string(out);
}


} // vnl

#endif /* INCLUDE_VNL_VAR_H_ */
