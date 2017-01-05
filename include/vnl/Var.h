/*
 * Var.h
 *
 *  Created on: Jan 5, 2017
 *      Author: mad
 */

#ifndef INCLUDE_VNL_VAR_H_
#define INCLUDE_VNL_VAR_H_

#include <vnl/String.h>
#include <vnl/List.h>
#include <vnl/Map.h>

#include <vnl/Value.hxx>


namespace vnl {

class Var {
public:
	enum {
		NIL, BOOL, INT, REAL, STRING, VALUE, LIST, MAP, PVAR
	};
	
	Var() : type(NIL) { mem_ = 0; }
	Var(const bool& v) : type(BOOL) { bool_ = v; }
	Var(const int64_t& v) : type(INT) { long_ = v; }
	Var(const double& v) : type(REAL) { double_ = v; }
	Var(const char* v) : type(NIL) { *this = String(v); }
	Var(const String& v) : type(NIL) { *this = v; }
	Var(const Value& v) : type(NIL) { *this = v; }
	Var(const List<Var>& v) : type(NIL) { *this = v; }
	Var(const Map<Var,Var>& v) : type(NIL) { *this = v; }
	Var(Var* v) : type(PVAR) { var_ = v; if(v) { v->ref_count++; } }
	
	Var(const Var& v) : type(NIL) {
		*this = v;
	}
	
	~Var() {
		if(ref_count) {
			// TODO: throw MemoryException();
			assert(false);
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
		long_ = v;
		return *this;
	}
	
	Var& operator=(uint16_t v) {
		clear();
		type = INT;
		long_ = v;
		return *this;
	}
	
	Var& operator=(uint32_t v) {
		clear();
		type = INT;
		long_ = v;
		return *this;
	}
	
	Var& operator=(uint64_t v) {
		clear();
		type = INT;
		long_ = v;
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
	
	Var& operator=(const List<Var>& v) {
		clear();
		type = LIST;
		if(v.size()) {
			list_ = new List<Var>(v);
		}
		return *this;
	}
	
	Var& operator=(const Map<Var,Var>& v) {
		clear();
		type = MAP;
		if(v.size()) {
			map_ = new Map<Var,Var>(v);
		}
		return *this;
	}
	
	Var& operator=(Var* v) {
		clear();
		type = PVAR;
		var_ = v;
		if(v) { v->ref_count++; }
		return *this;
	}
	
	Var& operator=(const Var& v) {
		clear();
		switch(v.type) {
			case BOOL: 		bool_ = v.bool_; break;
			case STRING: 	if(v.string_) { string_ = new String(*v.string_); } break;
			case LIST: 		if(v.list_) { list_ = new List<Var>(*v.list_); } break;
			case MAP: 		if(v.map_) { map_ = new Map<Var,Var>(*v.map_); } break;
			case PVAR: 		if(v.var_) { var_ = v.var_; v.var_->ref_count++; } break;
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
			}
			return mem_ == v.mem_;
		}
		char buf[128];
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
				case STRING:
					if(v.string_) {
						v.string_->to_string(buf, sizeof(buf));
						return long_ == ::atoll(buf);
					}
			}
			return false;
		case REAL:
			switch(v.type) {
				case BOOL: 		return bool(double_) == v.bool_;
				case INT: 		return int64_t(double_) == v.long_;
				case STRING:
					if(v.string_) {
						v.string_->to_string(buf, sizeof(buf));
						return double_ == ::atof(buf);
					}
			}
			return false;
		case STRING:
			if(string_) {
				switch(v.type) {
					case INT: 
						string_->to_string(buf, sizeof(buf));
						return ::atoll(buf) == v.long_;
					case REAL:
						string_->to_string(buf, sizeof(buf));
						return ::atof(buf) == v.double_;
				}
			}
			return false;
		}
		return false;
	}
	
	bool operator!=(const Var& v) const {
		return !(*this == v);
	}
	
	void to(bool& v) const {
		switch(type) {
			case BOOL: 		v = bool_; break;
			case INT: 		v = long_; break;
			case REAL: 		v = double_; break;
			case STRING: 	if(string_) { v = string_->size(); } else { v = false; } break;
			case LIST: 		if(list_) { v = list_->size(); } else { v = false; } break;
			case MAP: 		if(map_) { v = map_->size(); } else { v = false; } break;
			case PVAR: 		if(var_) { var_->to(v); } else { v = false; } break;
			default: 		v = false;
		}
	}
	
	void to(int32_t& v) const {
		// TODO
	}
	
	void to(int64_t& v) const {
		// TODO
	}
	
	void to(float& v) const {
		// TODO
	}
	
	void to(double& v) const {
		// TODO
	}
	
	void to(String& v) const {
		v.clear();
		switch(type) {
			case BOOL: 		vnl::to_string(v, bool_); break;
			case INT: 		vnl::to_string(v, long_); break;
			case REAL: 		vnl::to_string(v, double_); break;
			case STRING: 	if(string_) { v = *string_; } break;
			case LIST: 		if(list_) { vnl::to_string(v, *list_); } else { v = "[]"; } break;
			case MAP: 		if(map_) { vnl::to_string(v, *map_); } else { v = "{}"; } break;
			case PVAR: 		if(var_) { var_->to(v); } break;
		}
	}
	
	void to(Value& v) const {
		// TODO
	}
	
	void to(List<Var>& v) const {
		// TODO
	}
	
	void to(Map<Var,Var>& v) const {
		// TODO
	}
	
	template<typename T>
	void to(T& v) const {
		// nothing
	}
	
	operator bool() const { bool v; to(v); return v; }
	operator int8_t() const { int64_t v; to(v); return v; }
	operator int16_t() const { int64_t v; to(v); return v; }
	operator int32_t() const { int64_t v; to(v); return v; }
	operator int64_t() const { int64_t v; to(v); return v; }
	operator uint8_t() const { int64_t v; to(v); return v; }
	operator uint16_t() const { int64_t v; to(v); return v; }
	operator uint32_t() const { int64_t v; to(v); return v; }
	operator uint64_t() const { int64_t v; to(v); return v; }
	operator float() const { double v; to(v); return v; }
	operator double() const { double v; to(v); return v; }
	
	void clear() {
		switch(type) {
			case STRING: 	delete string_; break;
			case LIST: 		delete list_; break;
			case MAP: 		delete map_; break;
			case VALUE: 	vnl::destroy(value_); break;
			case PVAR: 		if(var_) { var_->ref_count--; } break;
		}
		mem_ = 0;
		type = NIL;
	}
	
	union {
		uint64_t mem_;
		bool bool_;
		int64_t long_;
		double double_;
		String* string_;
		Value* value_;
		List<Var>* list_;
		Map<Var,Var>* map_;
		Var* var_;
	};
	
	int type;
	int ref_count = 0;
	
};


inline uint64_t hash(const Var& obj) {
	switch(obj.type) {
		case Var::BOOL: 	return vnl::hash(obj.bool_);
		case Var::INT: 		return vnl::hash(obj.long_);
		case Var::REAL: 	return vnl::hash(obj.double_);
		case Var::STRING: 	if(obj.string_) { return vnl::hash(obj.string_); } break;
		case Var::PVAR: 	if(obj.var_) { return vnl::hash(*obj.var_); } break;
	}
	return 0;
}

inline void read(vnl::io::TypeInput& in, vnl::Var& obj) {
	obj.clear();
	int size = 0;
	int id = in.getEntry(size);
	switch(id) {
		case VNL_IO_NULL: break;
		case VNL_IO_BOOL: obj = Var(size == VNL_IO_TRUE); break;
		case VNL_IO_INTEGER: {
			obj.type = Var::INT;
			in.readValue(obj.long_, id, size);
			break;
		}
		case VNL_IO_REAL: {
			obj.type = Var::REAL;
			in.readValue(obj.double_, id, size);
			break;
		}
		case VNL_IO_STRING: {
			obj.type = Var::STRING;
			obj.string_ = new String();
			in.readString(*obj.string_, size);
			break;
		}
		// TODO: VNL_IO_ARRAY etc
		default: in.skip(id, size);
	}
}

inline void write(vnl::io::TypeOutput& out, const vnl::Var& obj) {
	switch(obj.type) {
		case Var::BOOL: 	vnl::write(out, obj.bool_); break;
		case Var::INT: 		vnl::write(out, obj.long_); break;
		case Var::REAL: 	vnl::write(out, obj.double_); break;
		case Var::STRING: 	if(obj.string_) { vnl::write(out, *obj.string_); } else { out.putNull(); } break;
		case Var::LIST: 	if(obj.list_) { vnl::write(out, *obj.list_); } else { out.putNull(); } break;
		case Var::MAP: 		if(obj.map_) { vnl::write(out, *obj.map_); } else { out.putNull(); } break;
		case Var::PVAR: 	if(obj.var_) { vnl::write(out, *obj.var_); } else { out.putNull(); } break;
		default: 			out.putNull();
	}
}

inline void from_string(const vnl::String& str, vnl::Var& obj) {
	obj = str;
}

inline void to_string(vnl::String& out, const vnl::Var& obj) {
	switch(obj.type) {
		case Var::BOOL: 	vnl::to_string(out, obj.bool_); break;
		case Var::INT: 		vnl::to_string(out, obj.long_); break;
		case Var::REAL: 	vnl::to_string(out, obj.double_); break;
		case Var::STRING: 	if(obj.string_) { vnl::to_string(out, *obj.string_); } else { out << "\"\""; } break;
		case Var::LIST: 	if(obj.list_) { vnl::to_string(out, *obj.list_); } else { out << "[]"; } break;
		case Var::MAP: 		if(obj.map_) { vnl::to_string(out, *obj.map_); } else { out << "{}"; } break;
		case Var::PVAR: 	if(obj.var_) { vnl::to_string(out, *obj.var_); } else { out << "{}"; } break;
		default: 			out << "{}";
	}
}


} // vnl

#endif /* INCLUDE_VNL_VAR_H_ */