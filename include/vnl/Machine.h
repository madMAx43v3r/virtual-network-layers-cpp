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
