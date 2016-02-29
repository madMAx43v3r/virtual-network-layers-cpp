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
#include <functional>

#include "io/Stream.h"

namespace vnl { namespace phy {

class Object;
class Fiber;

class Message {
public:
	Message() {}
	virtual ~Message() {}
	
	Message(uint32_t mid, uint64_t sid = 0, bool async = false)
		:	mid(mid), sid(sid), async(async) {}
	
	virtual std::string toString();
	
	void ack();
	
	Object* src = 0;
	Object* dst = 0;
	uint32_t mid = 0;
	uint64_t sid = 0;
	Fiber* impl = 0;
	bool isack = false;
	bool async = false;
	
	std::function<void(Message*)> callback;
	
};

template<uint32_t MID>
class Signal : public Message {
public:
	Signal() : Message() {}
	Signal(uint64_t sid, bool async = false) : Message(MID, sid, async) {}
	
	static const uint32_t id = MID;
	
private:
	template<typename X>
	Signal(Object* dst, X sid);
	
};

template<typename T, uint32_t MID>
class Generic : public Message {
public:
	Generic() : Message() {}
	Generic(const T& data, uint64_t sid = 0, bool async = false) : Message(MID, sid, async), data(data) {}
	
	static const uint32_t id = MID;
	
	T data;
	
private:
	template<typename X>
	Generic(const T& data, Object* dst, X sid);
	
};

template<typename T, typename P, uint32_t MID>
class Request : public Message {
public:
	Request() : Message() {}
	Request(const P& args, uint64_t sid = 0, bool async = false) : Message(MID, sid, async), args(args) {}
	
	void ack(const T& result) {
		res = result;
		Message::ack();
	}
	
	static const uint32_t id = MID;
	
	typedef T res_t;
	typedef P args_t;
	
	T res;
	P args;
	
private:
	template<typename X>
	Request(const P& args, Object* dst, X sid);
	
};



}}

#endif /* INCLUDE_PHY_MESSAGE_H_ */
