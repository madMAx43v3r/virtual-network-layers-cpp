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

#include "io/Stream.h"

namespace vnl { namespace phy {

class Object;

class Message {
public:
	Message() {}
	virtual ~Message() {}
	
	Message(Object* dst, uint32_t mid, uint32_t oid = 0, uint64_t sid = 0, bool async = false)
		:	dst(dst), mid(mid), oid(oid), sid(sid), async(async) {}
	
	virtual std::string toString();
	
	virtual bool serialize(vnl::io::Stream*) { return false; }
	virtual bool deserialize(vnl::io::Stream*) { return false; }
	
	template<typename T>
	T* cast();
	
	template<typename T>
	bool is();
	
	void ack();
	
	Object* src = 0;
	Object* dst = 0;
	uint32_t mid = 0;
	uint32_t oid = 0;
	uint64_t sid = 0;
	void* impl = 0;
	bool isack = false;
	bool async = false;
	
};

template<uint32_t MID, uint32_t OID = 0>
class Signal : public Message {
public:
	Signal() : Message() {}
	Signal(Object* dst, uint64_t sid = 0, bool async = false) : Message(dst, MID, OID, sid, async) {}
	
	static const uint32_t mid = MID;
	static const uint32_t oid = OID;
	
};

template<typename T, uint32_t MID, uint32_t OID = 0>
class Generic : public Message {
public:
	Generic() : Message() {}
	Generic(const T& data, Object* dst, uint64_t sid = 0, bool async = false) : Message(dst, MID, OID, sid, async), data(data) {}
	
	static const uint32_t mid = MID;
	static const uint32_t oid = OID;
	
	T data;
	
};

template<typename T, typename P, uint32_t MID, uint32_t OID = 0>
class Request : public Message {
public:
	Request() : Message() {}
	Request(const P& args, Object* dst, uint64_t sid = 0, bool async = false) : Message(dst, MID, OID, sid, async), args(args) {}
	
	void ack(const T& result) {
		res = result;
		Message::ack();
	}
	
	static const uint32_t mid = MID;
	static const uint32_t oid = OID;
	
	typedef T res_t;
	typedef P args_t;
	
	T res;
	P args;
	
};


template<typename T>
T* Message::cast() {
	if(mid == T::mid) {
		return (T*)this;
	} else {
		return 0;
	}
}

template<typename T>
bool Message::is() {
	return mid == T::mid;
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
	ss << "[" << Util::demangle(this) << "] mid=0x" << std::hex << mid << " oid=0x" << oid << std::dec
			<< " src=" << src << " dst=" << dst << " sid=" << sid << " isack=" << isack << " async=" << async;
	return ss.str();
}


}}

#endif /* INCLUDE_PHY_MESSAGE_H_ */
