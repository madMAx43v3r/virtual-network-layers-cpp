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
	
	Message(Object* dst, uint32_t mid, uint64_t sid = 0, bool async = false)
		:	dst(dst), mid(mid), seq(0), sid(sid), async(async) {}
	
	virtual std::string toString();
	
	void ack();
	
	Object* src = 0;
	Object* dst = 0;
	uint32_t mid = 0;
	uint32_t seq = 0;
	uint64_t sid = 0;
	void* impl = 0;
	bool isack = false;
	bool async = false;
	
	std::function<void(Message*)> callback;
	
};

template<uint32_t MID>
class Signal : public Message {
public:
	Signal() : Message() {}
	Signal(Object* dst, uint64_t sid = 0, bool async = false) : Message(dst, MID, sid, async) {}
	
	static const uint32_t mid = MID;
	
};

template<typename T, uint32_t MID>
class Generic : public Message {
public:
	Generic() : Message() {}
	Generic(const T& data, Object* dst, uint64_t sid = 0, bool async = false) : Message(dst, MID, sid, async), data(data) {}
	
	static const uint32_t mid = MID;
	
	T data;
	
};

template<typename T, typename P, uint32_t MID>
class Request : public Message {
public:
	Request() : Message() {}
	Request(const P& args, Object* dst, uint64_t sid = 0, bool async = false) : Message(dst, MID, sid, async), args(args) {}
	
	void ack(const T& result) {
		res = result;
		Message::ack();
	}
	
	static const uint32_t mid = MID;
	
	typedef T res_t;
	typedef P args_t;
	
	T res;
	P args;
	
};


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
	ss << "[" << Util::demangle(this) << "] mid=0x" << std::hex << mid << std::dec
			<< " src=" << src << " dst=" << dst << " sid=" << sid << " isack=" << isack << " async=" << async;
	return ss.str();
}


}}

#endif /* INCLUDE_PHY_MESSAGE_H_ */
