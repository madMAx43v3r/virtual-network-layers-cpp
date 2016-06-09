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

#include "vnl/RingBuffer.h"

#define VNL_MSG(type, hash) typedef vnl::MessageType<type, hash> msg_t;


namespace vnl {

class Base;
class Engine;


class Message {
public:
	Message() : msg_id(0) {}
	
	Message(uint32_t mid) : msg_id(mid) {}
	
	virtual ~Message() {}
	
	Message(const Message&) = delete;
	Message& operator=(const Message&) = delete;
	
	virtual std::string to_string();
	
	void ack();
	
	void release() {
		if(buffer) {
			buffer->destroy<Message>(entry);
		}
	}
	
	uint32_t msg_id;
	Base* src = 0;
	Base* dst = 0;
	bool isack = false;
	
	RingBuffer* buffer = 0;
	RingBuffer::entry_t* entry = 0;
	
	void* user = 0;
	std::function<void(Message*)>* callback = 0;
	
	Base* gate = 0;
	void* _impl = 0;
	
};


template<uint32_t MID_>
class SignalType : public Message {
public:
	SignalType() : Message(MID_) {}
	
	static const uint32_t MID = MID_;
	
};


template<typename T, uint32_t MID_>
class MessageType : public Message {
public:
	MessageType() : Message(MID_) {}
	
	template<typename R>
	MessageType(R&& data) : Message(MID_), data(data) {}
	
	static const uint32_t MID = MID_;
	
	T data;
	
	typedef T data_t;
	
};


template<typename T, typename P, uint32_t MID_>
class RequestType : public Message {
public:
	RequestType() : Message(MID_) {}
	
	template<typename R>
	RequestType(R&& args) : Message(MID_), args(args) {}
	
	void ack(const T& result) {
		res = result;
		Message::ack();
	}
	
	static const uint32_t MID = MID_;
	
	typedef T res_t;
	typedef P args_t;
	
	T res;
	P args;
	
};



}

#endif /* INCLUDE_PHY_MESSAGE_H_ */
