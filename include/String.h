/*
 * String.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_STRING_H_
#define INCLUDE_PHY_STRING_H_

#include <string.h>
#include <ostream>
#include <streambuf>
#include <string>
#include <sstream>

#include "phy/Pool.h"


namespace vnl {

class String : public std::ostream, private std::streambuf {
public:
	static const int CHUNK_SIZE = VNL_STRING_BLOCK_SIZE - sizeof(void*) - sizeof(short);
	
	struct chunk_t {
		char str[CHUNK_SIZE];
		chunk_t* next = 0;
		short len = 0;
	};
	
	static phy::AtomicPool<chunk_t>* chunks;
	
	String() : std::ostream(this) {}
	
	String(const char* str) : std::ostream(this) {
		*this << str;
	}
	
	String(const std::string& str) : std::ostream(this) {
		*this << str;
	}
	
	String(String& str) : std::ostream(this) {
		*this << str;
	}
	
	~String() {
		clear();
	}
	
	String& operator=(String& str) {
		clear();
		*this << str;
		return *this;
	}
	
	String& operator<<(String& str) {
		str.sync();
		chunk_t* chunk = str.p_front;
		while(chunk) {
			std::ostream::write(chunk->str, chunk->len);
			chunk = chunk->next;
		}
		return *this;
	}
	
	std::string to_string() {
		sync();
		std::ostringstream stream;
		chunk_t* chunk = p_front;
		while(chunk) {
			stream.write(chunk->str, chunk->len);
			chunk = chunk->next;
		}
		return stream.str();
	}
	
	friend std::ostream& operator<<(std::ostream& stream, String& str) {
		str.sync();
		chunk_t* chunk = str.p_front;
		while(chunk) {
			stream.write(chunk->str, chunk->len);
			chunk = chunk->next;
		}
		return stream;
	}
	
	void clear();
	
protected:
	virtual int overflow(int c = std::char_traits<char>::eof()) override;
	
	virtual int sync() override;
	
	void push_back(chunk_t* chunk);
	
private:
	chunk_t* p_front = 0;
	chunk_t* p_back = 0;
	chunk_t* buf = 0;
	
};


} // vnl




#endif /* INCLUDE_PHY_STRING_H_ */
