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
#include <atomic>

#include "vnl/phy/Memory.h"


namespace vnl {

class String {
public:
	static const int CHUNK_SIZE = VNL_STRING_BLOCK_SIZE - sizeof(void*) - sizeof(short);
	
	struct chunk_t {
		char str[CHUNK_SIZE];
		chunk_t* next = 0;
		int16_t len = 0;
	};
	
	String(phy::Region& memory) : memory(memory) {
		p_front = memory.create<chunk_t>();
		p_back = p_front;
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
	
	bool operator==(const std::string& other) const {
		// TODO
		return false;
	}
	
	bool operator==(const char* other) const {
		// TODO
		return false;
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
	
	std::string to_string() const {
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
	
	void write(const char* str, int32_t len) {
		int32_t pos = 0;
		while(len > pos) {
			if(p_back->len == CHUNK_SIZE) {
				if(!p_back->next) {
					p_back->next = memory.create<chunk_t>();
				}
				p_back = p_back->next;
			}
			int32_t num = len - pos;
			int32_t left = CHUNK_SIZE - p_back->len;
			if(num > left) { num = left; }
			memcpy(p_back->str + p_back->len, str+pos, num);
			p_back->len += num;
			pos += num;
		}
	}
	
	void clear() {
		chunk_t* chunk = p_front;
		while(chunk) {
			chunk->len = 0;
			chunk = chunk->next;
		}
		p_back = p_front;
		count = 0;
	}
	
	int32_t size() const {
		return count;
	}
	
	chunk_t* front() const {
		return p_front;
	}
	
	chunk_t* back() const {
		return p_back;
	}
	
private:
	phy::Region& memory;
	chunk_t* p_front;
	chunk_t* p_back;
	int32_t count = 0;
	
	static std::atomic<chunk_t*> begin;
	
};


} // vnl




#endif /* INCLUDE_PHY_STRING_H_ */
