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
#include <mutex>

#include "vnl/Memory.h"


namespace vnl {

class ToString {
public:
	static const int BUF_SIZE = 128;
	char buf[BUF_SIZE];
	int len = 0;
};


class String {
public:
	static const int CHUNK_SIZE = VNL_STRING_BLOCK_SIZE - sizeof(void*) - 2;
	
	struct chunk_t {
		char str[CHUNK_SIZE];
		chunk_t* next;
		int16_t len;
		chunk_t() : next(0), len(0) {}
	};
	
	String(Region& memory) : memory(memory) {
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
		assert(false);
		return false;
	}
	
	bool operator==(const char* other) const {
		// TODO
		assert(false);
		return false;
	}
	
	String& operator=(const char* str) {
		clear();
		return *this << str;
	}
	
	String& operator=(const std::string& str) {
		clear();
		return *this << str;
	}
	
	String& operator=(const String& str) {
		clear();
		return *this << str;
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
	
	String& operator<<(const ToString& str) {
		write(str.buf, str.len);
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
	Region& memory;
	chunk_t* p_front = 0;
	chunk_t* p_back = 0;
	int32_t count = 0;
	
};


class str : public ToString {
public:
	str(const char src[]) {
		len = strlen(src);
		if(len > BUF_SIZE) {
			len = BUF_SIZE;
		}
		strncpy(buf, src, len);
	}
};

static str endl("\n");

class dec : public ToString {
public:
	dec(int32_t i)  { len = snprintf(buf, BUF_SIZE, "%d", i); }
	dec(uint32_t i) { len = snprintf(buf, BUF_SIZE, "%u", i); }
	dec(int64_t i)  { len = snprintf(buf, BUF_SIZE, "%ld", i); }
	dec(uint64_t i) { len = snprintf(buf, BUF_SIZE, "%lu", i); }
};

class hex : public ToString {
public:
	hex(int32_t i)  { len = snprintf(buf, BUF_SIZE, "0x%x", i); }
	hex(uint32_t i) { len = snprintf(buf, BUF_SIZE, "0x%x", i); }
	hex(int64_t i)  { len = snprintf(buf, BUF_SIZE, "0x%lx", i); }
	hex(uint64_t i) { len = snprintf(buf, BUF_SIZE, "0x%lx", i); }
};

class def : public ToString {
public:
	def(float f, int precision = 6)  { len = snprintf(buf, BUF_SIZE, "%.*f", precision, f); }
	def(double f, int precision = 6) { len = snprintf(buf, BUF_SIZE, "%.*f", precision, f); }
};

class sci : public ToString {
public:
	sci(float f, int precision = 6)  { len = snprintf(buf, BUF_SIZE, "%.*e", precision, f); }
	sci(double f, int precision = 6) { len = snprintf(buf, BUF_SIZE, "%.*e", precision, f); }
};

class fix : public ToString {
public:
	fix(float f, int precision = 6)  { len = snprintf(buf, BUF_SIZE, "%0*f", precision, f); }
	fix(double f, int precision = 6) { len = snprintf(buf, BUF_SIZE, "%0*f", precision, f); }
};


class StringOutput {
public:
	virtual ~StringOutput() {}
	virtual void write(const String& str) = 0;
};

class StringStream : public StringOutput {
public:
	StringStream(std::ostream& stream) : stream(stream) {}
	virtual void write(const String& str) {
		mutex.lock();
		stream << str;
		mutex.unlock();
	}
private:
	std::ostream& stream;
	std::mutex mutex;
};

extern StringStream cout;
extern StringStream cerr;


class StringWriter {
public:
	StringWriter() : out(memory), func(nullptr) {}
	StringWriter(StringOutput* func) : out(memory), func(func) {}
	StringWriter(const StringWriter& other) : out(memory), func(other.func) {}
	~StringWriter() {
		if(func) {
			func->write(out);
		}
	}
private:
	Region memory;
	StringOutput* func;
public:
	String out;
};





} // vnl

#endif /* INCLUDE_PHY_STRING_H_ */
