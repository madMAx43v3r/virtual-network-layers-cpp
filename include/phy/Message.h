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

class Node;
class Engine;
class Fiber;


class Message {
public:
	Message() : mid(0) {}
	
	Message(uint32_t mid) : mid(mid) {}
	
	virtual ~Message() {}
	
	virtual std::string toString();
	
	void ack();
	
	uint32_t mid;
	Node* src = 0;
	Node* dst = 0;
	bool isack = false;
	
	void* user = 0;
	std::function<void(Message*)> callback;
	
	Node* gate = 0;
	Fiber* _impl = 0;
	
protected:
	Message(const Message&) = default;
	Message& operator=(const Message&) = default;
	
	friend class Engine;
	
};


template<uint32_t MID>
class Signal : public Message {
public:
	Signal() : Message(MID) {}
	
	static const uint32_t id = MID;
	
};


template<typename T, uint32_t MID>
class Generic : public Message {
public:
	Generic() : Message(MID) {}
	
	template<typename R>
	Generic(R&& data) : Message(MID), data(data) {}
	
	static const uint32_t id = MID;
	
	T data;
	
};


template<typename T, typename P, uint32_t MID>
class Request : public Message {
public:
	Request() : Message(MID) {}
	
	template<typename R>
	Request(R&& args) : Message(MID), args(args) {}
	
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
