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
#include <string>
#include <sstream>

#include "phy/Pool.h"


namespace vnl {

class String {
public:
	static const int CHUNK_SIZE = VNL_STRING_BLOCK_SIZE - sizeof(void*) - sizeof(short);
	
	struct chunk_t {
		char str[CHUNK_SIZE];
		chunk_t* next = 0;
		short len = 0;
	};
	
	static phy::AtomicPool<chunk_t>* chunks;
	
	String() {}
	
	String(const char* str) {
		*this << str;
	}
	
	String(const std::string& str) {
		*this << str;
	}
	
	String(const String& str) {
		*this << str;
	}
	
	~String() {
		clear();
	}
	
	bool operator!=(const String& other) const {
		return !(*this == other);
	}
	
	bool operator==(const String& other) const {
		chunk_t* A = p_front;
		chunk_t* B = other.p_front;
		while(A && B) {
			if(A->len != B->len) {
				return false;
			}
			if(strncmp(A->str, B->str, A->len)) {
				return false;
			}
			A = A->next;
			B = B->next;
		}
		return A == B;
	}
	
	String& operator=(const String& str) {
		clear();
		*this << str;
		return *this;
	}
	
	String& operator<<(const char* str) {
		write(str, strlen(str));
		return *this;
	}
	
	String& operator<<(const std::string& str) {
		write(str.c_str(), str.size());
		return *this;
	}
	
	String& operator<<(const String& str) {
		chunk_t* chunk = str.p_front;
		while(chunk) {
			write(chunk->str, chunk->len);
			chunk = chunk->next;
		}
		return *this;
	}
	
	std::string to_string() {
		std::ostringstream stream;
		chunk_t* chunk = p_front;
		while(chunk) {
			stream.write(chunk->str, chunk->len);
			chunk = chunk->next;
		}
		return stream.str();
	}
	
	friend std::ostream& operator<<(std::ostream& stream, const String& str) {
		chunk_t* chunk = str.p_front;
		while(chunk) {
			stream.write(chunk->str, chunk->len);
			chunk = chunk->next;
		}
		return stream;
	}
	
	chunk_t* front() { return p_front; }
	chunk_t* back() { return p_back; }
	
	const chunk_t* front() const { return p_front; }
	const chunk_t* back() const { return p_back; }
	
	void write(const char* str, size_t len);
	
	void clear();
	
protected:
	void push_back(chunk_t* chunk);
	
private:
	chunk_t* p_front = 0;
	chunk_t* p_back = 0;
	
};


} // vnl




#endif /* INCLUDE_PHY_STRING_H_ */
