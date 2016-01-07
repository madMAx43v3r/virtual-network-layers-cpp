/*
 * Message.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_MESSAGE_H_
#define INCLUDE_PHY_MESSAGE_H_

#include <stdint.h>
#include <iostream>
#include <sstream>

namespace vnl { namespace phy {

class Object;

class Message {
public:
	Message() {}
	virtual ~Message() {}
	
	Message(uint32_t type, bool async = false) : type(type), async(async) {}
	Message(Object* dst, uint32_t type) : type(type), dst(dst) {}
	Message(Object* dst, uint64_t sid, uint32_t type) : type(type), dst(dst), sid(sid) {}
	
	virtual std::string toString();
	
	template<typename T>
	T* cast();
	
	void ack();
	
	uint32_t type = 0;
	Object* src = 0;
	Object* dst = 0;
	uint64_t sid = 0;
	void* impl = 0;
	bool isack = false;
	bool async = false;
};

template<typename T, uint32_t N>
class Generic : public Message {
public:
	Generic() : Message() {}
	Generic(const T& obj, bool async = false) : Message(N, async), data(obj) {}
	Generic(Object* dst, const T& obj) : Message(dst, N), data(obj) {}
	Generic(Object* dst, uint64_t sid, const T& obj) : Message(dst, sid, N), data(obj) {}
	
	static const uint32_t id = N;
	
	T data;
	
};

template<typename T, typename P, uint32_t N>
class Request : public Message {
public:
	Request(const P& args, bool async = false) : Message(N, async), args(args) {}
	Request(Object* dst, const P& args) : Message(dst, N), args(args) {}
	
	void ack(const T& result) {
		res = result;
		Message::ack();
	}
	
	static const uint32_t id = N;
	
	typedef T res_t;
	typedef P args_t;
	
	T res;
	P args;
	
};


template<typename T>
T* Message::cast() {
	if(type == T::id) {
		return (T*)this;
	} else {
		return 0;
	}
}

void Message::ack() {
	if(!isack) {
		isack = true;
		if(src) {
			src->receive(this, dst);
		} else if(async) {
			delete this;
		}
	}
}

std::string Message::toString() {
	std::ostringstream ss;
	ss << "[" << Util::demangle(this) << "] type=0x" << std::hex << type << std::dec
			<< " src=" << src << " dst=" << dst << " sid=" << sid << " isack=" << isack << " async=" << async;
	return ss.str();
}


}}

#endif /* INCLUDE_PHY_MESSAGE_H_ */
