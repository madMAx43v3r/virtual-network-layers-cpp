/*
 * Frame.h
 *
 *  Created on: 16.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_FRAME_H_
#define INCLUDE_FRAME_H_

#include "Address.h"
#include "phy/Message.h"

namespace vnl {

class Frame : public vnl::phy::Message {
public:
	
	static const uint32_t id = 0xf6b245f5;
	
	static const char NONE = 0x00;
	static const char UNICAST = 0x01;
	static const char ANYCAST = 0x02;
	static const char MULTICAST = 0x04;
	
	static const char REGISTER = 0x10 | MULTICAST;
	static const char UNREGISTER = 0x20 | MULTICAST;
	
	char flags;
	Address src;
	Address dst;
	uint64_t sid;
	
	char* data;
	int size;
	
	Frame() {
		type = id;
		flags = NONE;
		sid = 0;
		data = 0;
		size = 0;
	}
	
	~Frame() {
		delete data;
	}
	
	Frame(char flags, const Address& dst) {
		type = id;
		this->flags = flags;
		this->dst = dst;
		sid = 0;
		data = 0;
		size = 0;
	}
	
	Frame(char flags, const Address& dst, int size) {
		type = id;
		this->flags = flags;
		this->dst = dst;
		this->data = new char[size];
		this->size = size;
		sid = 0;
	}
	
	Message* base() {
		return this;
	}
	
	bool isNull() {
		return flags == NONE;
	}
	
};

}

#endif /* INCLUDE_FRAME_H_ */
