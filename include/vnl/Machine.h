/*
 * Machine.h
 *
 *  Created on: Dec 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_MACHINE_H_
#define INCLUDE_VNL_MACHINE_H_

#include <vnl/Type.hxx>


namespace vnl {

class Function {
public:
	
	
};

class Var {
public:
	enum {
		NIL, INT, REAL, STRING, LIST, MAP, VAR, SYM
	};
	
	Var() : type(NIL) { mem_ = 0; }
	Var(const int64_t& v) : type(INT) { long_ = v; }
	Var(const double& v) : type(REAL) { double_ = v; }
	Var(String* v) : type(STRING) { string_ = v; }
	Var(Var* v) : type(VAR) { var_ = v; }
	
	bool operator==(const Var& v) const {
		if(type == v.type) {
			switch(type) {
				case NIL: return true;
				case STRING: return string_ == v.string_;
			}
			return mem_ == v.mem_;
		}
		char buf[128];
		switch(type) {
		case INT:
			switch(v.type) {
			case REAL: return long_ == int64_t(v.double_);
			case STRING:
				v.string_->to_string(buf, sizeof(buf));
				return long_ == ::atoll(buf);
			}
			return false;
		case REAL:
			switch(v.type) {
			case INT: return int64_t(double_) == v.long_;
			case STRING:
				v.string_->to_string(buf, sizeof(buf));
				return double_ == ::atof(buf);
			}
			return false;
		case STRING:
			switch(v.type) {
			case INT: 
				string_->to_string(buf, sizeof(buf));
				return ::atoll(buf) == v.long_;
			case REAL:
				string_->to_string(buf, sizeof(buf));
				return ::atof(buf) == v.double_;
			}
			return false;
		}
		return false;
	}
	
	union {
		uint64_t mem_;
		int64_t long_;
		double double_;
		String* string_;
		List<Var>* list_;
		Map<Var,Var>* map_;
		Var* var_;
		uint64_t sym_;
	};
	
	int type;
	bool locked = false;
	
};


class stack_overflow : public std::exception {
public:
	virtual const char* what() const {
		return "vnl::stack_overflow";
	}
};

class segmentation_fault : public std::exception {
public:
	virtual const char* what() const {
		return "vnl::segmentation_fault";
	}
};

class memory_error : public std::exception {
public:
	virtual const char* what() const {
		return "vnl::memory_error";
	}
};


class Machine {
public:
	enum {
		NOOP, MOV, CPY, CMP, ADD, SUB, MUL, DIV, MOD, PUSH, POP,
		READ, LOAD, STORE, DELETE, SHIFT_L, SHIFT_R, CALL, RET
	};
	
private:
	struct op_t {
		int code = 0;
		int a = 0;
		int b = 0;
		int c = 0;
	};
	
public:
	Machine(int stack_size = 256)
		:	stack_size(stack_size)
	{
		p_stack = new Var[stack_size];
	}
	
	~Machine() {
		
	}
	
protected:
	Var& stack(int i) {
		if(i < 0) {
			i = sp + i;
		}
		if(i < 0 || i >= stack_size) {
			throw segmentation_fault();
		}
		return p_stack[i];
	}
	
	op_t code(int i) {
		
	}
	
private:
	int sp = 0;
	int stack_size;
	Var* p_stack;
	
	int ip = 0;
	int code_size;
	const char* p_code;
	
	Array<Var> memory;
	Queue<Var*> free_list;
	
	Pool<String> string_pool;
	Pool<List<Var> > list_pool;
	Pool<Map<Var,Var> > map_pool;
	
};


} // vnl

#endif /* INCLUDE_VNL_MACHINE_H_ */
