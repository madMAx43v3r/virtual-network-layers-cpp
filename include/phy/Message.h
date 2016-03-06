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

namespace vnl { namespace phy {

class Stream;
class Fiber;

class Message {
public:
	Message() {}
	virtual ~Message() {}
	
	Message(uint32_t mid, bool async = false)
		:	mid(mid), async(async) {}
	
	virtual std::string toString();
	
	void ack();
	
	uint32_t mid = 0;
	Stream* src = 0;
	Stream* dst = 0;
	Fiber* impl = 0;
	bool isack = false;
	bool async = false;
	
	std::function<void(Message*)> callback;
	
};

template<uint32_t MID>
class Signal : public Message {
public:
	Signal() : Message() {}
	Signal(bool async) : Message(MID, async) {}
	
	static const uint32_t id = MID;
	
};

template<typename T, uint32_t MID>
class Generic : public Message {
public:
	Generic() : Message() {}
	Generic(const T& data, bool async = false) : Message(MID, async), data(data) {}
	
	static const uint32_t id = MID;
	
	T data;
	
};

template<typename T, typename P, uint32_t MID>
class Request : public Message {
public:
	Request() : Message() {}
	Request(const P& args, bool async = false) : Message(MID, async), args(args) {}
	
	void ack(const T& result) {
		res = result;
		Message::ack();
	}
	
	static const uint32_t id = MID;
	
	typedef T res_t;
	typedef P args_t;
	
	T res;
	P args;
	
};



}}

#endif /* INCLUDE_PHY_MESSAGE_H_ */
