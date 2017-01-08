/*
 * Machine.h
 *
 *  Created on: Dec 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_MACHINE_H_
#define INCLUDE_VNL_MACHINE_H_

#include <vnl/Function.hxx>
#include <vnl/StackOverflow.hxx>
#include <vnl/SegmentationFault.hxx>
#include <vnl/IllegalInstruction.hxx>
#include <vnl/IOException.hxx>


namespace vnl {

class Machine {
public:
	Machine(int stack_size = 256)
		:	stack_size(stack_size)
	{
		p_stack = new Var[stack_size];
	}
	
	~Machine() {
		delete [] p_stack;
	}
	
	void include(const Function* func) {
		functions[func->name] = func;
	}
	
	void write(int index, const Var& var) {
		if(++index > -num_vars) {
			throw IOException();
		}
		stack(sp - index) = var;
	}
	
	Var read(int index) {
		if(++index > num_vars) {
			throw IOException();
		}
		return stack(sp - index);
	}
	
	void push(const Var& var) {
		stack(sp++) = var;
	}
	
	Var& push() {
		return stack(sp++);
	}
	
	Var pop() {
		return stack(--sp);
	}
	
	int call(const Hash32& name) {
		const Function* func = functions.find(name);
		if(!func) {
			throw NoSuchMethodException();
		}
		return exec(func);
	}
	
	int exec(const Function* func) {
		int num_args = func->params.size();
		if(sp < num_args) {
			int i = 0;
			for(const Var& def : func->defaults) {
				if(i >= sp) {
					stack(i) = def;
				}
				i++;
			}
		}
		call_stack.clear();
		sp = num_args;
		fp = 0;
		call(func);
		while(exec());
		return num_vars;
	}
	
	int resume() {
		ip++;
		while(exec());
		return num_vars;
	}
	
protected:
	struct frame_t {
		const Function* fp = 0;
		List<code_t>::const_iterator ip;
	};
	
	bool exec() {
		if(ip == fp->code.end()) {
			throw SegmentationFault();
		}
		
		const code_t& code = *ip;
		int a = code.arg[0].long_;
		int b = code.arg[1].long_;
		
		switch(code.op) {
			case op_code_t::NOOP:	break;
			case op_code_t::MOV:	stack(a) = stack(b); break;
			case op_code_t::CMP:	push() = stack(a) == stack(b); break;
			case op_code_t::ADD:	push() = stack(a) + stack(b); break;
			case op_code_t::SUB:	push() = stack(a) - stack(b); break;
			case op_code_t::MUL:	push() = stack(a) * stack(b); break;
			case op_code_t::DIV:	push() = stack(a) / stack(b); break;
			case op_code_t::PUSH:	push() = code.arg[0]; break;
			case op_code_t::POP:	stack(--sp).clear(); break;
			case op_code_t::LOAD:	push() = stack(a).get_field(code.arg[1]); break;
			case op_code_t::STORE:	stack(a).set_field(code.arg[1], pop()); break;
			case op_code_t::JMP:	jump(a); return true;
			case op_code_t::CALL: {
				const Function* func = functions.find(stack(a));
				if(!func) {
					throw NoSuchMethodException();
				}
				int num_args = func->params.size();
				if(b > num_args) {
					sp -= b - num_args;
				}
				if(b < num_args) {
					int i = 0;
					for(const Var& def : func->defaults) {
						if(i >= b) {
							push(def);
						}
						i++;
					}
				}
				call(func);
				return true;
			}
			case op_code_t::RET: {
				frame_t frame = call_stack.pop_back();
				if(!frame.fp) {
					num_vars = sp;
					return false;
				}
				fp = frame.fp;
				ip = frame.ip;
				break;
			}
			case op_code_t::READ:
				for(int i = 0; i < a; ++i) {
					push().clear();
				}
				num_vars = -a;
				return false;
			case op_code_t::WRITE:
				num_vars = a;
				return false;
			default:
				throw IllegalInstruction();
		}
		
		ip++;
		return true;
	}
	
	void call(const Function* func) {
		frame_t& frame = *call_stack.push_back();
		frame.fp = fp;
		frame.ip = ip;
		fp = func;
		ip = fp->code.begin();
	}
	
	void jump(int offset) {
		if(offset > 0) {
			for(int i = 0; i < offset; ++i) {
				++ip;
				if(ip == fp->code.end()) {
					throw SegmentationFault();
				}
			}
		} else if(offset < 0) {
			for(int i = offset; i < 0; ++i) {
				if(ip == fp->code.begin()) {
					throw SegmentationFault();
				}
				--ip;
			}
		}
	}
	
	Var& stack(int i) {
		if(i < 0) {
			i = sp + i;
		}
		if(i < 0) {
			throw SegmentationFault();
		}
		if(i >= stack_size) {
			throw StackOverflow();
		}
		return p_stack[i];
	}
	
private:
	int sp = 0;
	int stack_size;
	Var* p_stack;
	
	const Function* fp = 0;
	List<code_t>::const_iterator ip;
	List<frame_t> call_stack;
	
	int num_vars = 0;
	
	Map<Hash32, const Function*> functions;
	
};


} // vnl

#endif /* INCLUDE_VNL_MACHINE_H_ */
