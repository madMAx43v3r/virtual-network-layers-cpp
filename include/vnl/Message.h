/*
 * Message.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_MESSAGE_H_
#define INCLUDE_PHY_MESSAGE_H_

#include <stdint.h>
#include <vnl/String.h>
#include <vnl/Pool.h>


namespace vnl {

class Basic;


class Message {
public:
	Message() : msg_id(0) {}
	
	Message(uint32_t mid) : msg_id(mid) {}
	
	virtual ~Message() {}
	
	Message(const Message&) = delete;
	Message& operator=(const Message&) = delete;
	
	virtual vnl::String to_string();
	
	void ack();
	void destroy();
	
	uint32_t msg_id;
	Basic* src = 0;
	Basic* dst = 0;
	bool isack = false;
	
	GenericPool* buffer = 0;
	int msg_size = 0;
	
	Basic* gate = 0;
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
