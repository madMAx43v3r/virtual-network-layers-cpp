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
#include <sstream>
#include <string>

#include <vnl/Memory.h>


namespace vnl {

class ToString {
public:
	static const int BUF_SIZE = 128;
	char buf[BUF_SIZE];
	int len = 0;
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


class String {
public:
	static const int CHUNK_SIZE = VNL_BLOCK_SIZE - 2;
	
	class chunk_t : public Block {
	public:
		chunk_t*& next_chunk() { return *((chunk_t**)(&next)); }
		uint16_t& len() { return type_at<uint16_t>(0); }
		char* str() { return mem + 2; }
		chunk_t* create() {
			len() = 0;
			return this;
		}
	};
	
	String() {
		p_front = Block::alloc_ex<chunk_t>()->create();
		p_back = p_front;
	}
	
	String(const char* str) : String() {
		(*this) << str;
	}
	
	String(const std::string& str) : String() {
		(*this) << str;
	}
	
	String(const String& other) : String() {
		chunk_t* chunk = other.p_front;
		while(chunk) {
			memcpy(p_back->str(), chunk->str(), chunk->len());
			p_back->len() = chunk->len();
			chunk = chunk->next_chunk();
			if(chunk) {
				p_back->next_chunk() = Block::alloc_ex<chunk_t>()->create();
				p_back = p_back->next_chunk();
			}
		}
	}
	
	~String() {
		p_front->free_all();
	}
	
	bool operator!=(const String& other) const {
		return !(*this == other);
	}
	
	bool operator==(const String& other) const {
		chunk_t* A = p_front;
		chunk_t* B = other.p_front;
		while(A && B) {
			if(A->len() != B->len()) {
				return false;
			}
			if(strncmp(A->str(), B->str(), A->len())) {
				return false;
			}
			A = A->next_chunk();
			B = B->next_chunk();
		}
		return A == 0 && B == 0;
	}
	
	bool operator==(const std::string& other) const {
		return *this == other.c_str();
	}
	
	bool operator==(const char* other) const {
		int off = 0;
		chunk_t* chunk = p_front;
		while(chunk) {
			if(strncmp(chunk->str(), other + off, chunk->len())) {
				return false;
			}
			off += chunk->len();
			chunk = chunk->next_chunk();
		}
		return chunk == 0;
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
	
	String& operator<<(const void* p) {
		return *this << vnl::hex((uint64_t)p);
	}
	
	String& operator<<(const int32_t& i) {
		return *this << vnl::dec(i);
	}
	
	String& operator<<(const int64_t& i) {
		return *this << vnl::dec(i);
	}
	
	String& operator<<(const uint32_t& i) {
		return *this << vnl::dec(i);
	}
	
	String& operator<<(const uint64_t& i) {
		return *this << vnl::dec(i);
	}
	
	String& operator<<(const float& f) {
		return *this << vnl::def(f);
	}
	
	String& operator<<(const double& d) {
		return *this << vnl::def(d);
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
			write(chunk->str(), chunk->len());
			chunk = chunk->next_chunk();
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
			stream.write(chunk->str(), chunk->len());
			chunk = chunk->next_chunk();
		}
		return stream.str();
	}
	
	int to_string(char* str, int len) const {
		chunk_t* chunk = p_front;
		int left = len-1;
		while(chunk && left > 0) {
			int n = std::min((int)chunk->len(), left);
			if(n <= 0) {
				break;
			}
			memcpy(str, chunk->str(), n);
			chunk = chunk->next_chunk();
			str += n;
			left -= n;
		}
		int num_bytes = len - left;
		str[num_bytes] = 0;
		return num_bytes + 1;
	}
	
	friend std::ostream& operator<<(std::ostream& stream, const String& str) {
		chunk_t* chunk = str.p_front;
		while(chunk) {
			stream.write(chunk->str(), chunk->len());
			chunk = chunk->next_chunk();
		}
		return stream;
	}
	
	void write(const char* str, int len) {
		int pos = 0;
		while(len > pos) {
			if(p_back->len() == CHUNK_SIZE) {
				if(!p_back->next_chunk()) {
					p_back->next_chunk() = Block::alloc_ex<chunk_t>()->create();
				}
				p_back = p_back->next_chunk();
			}
			int num = len - pos;
			int left = CHUNK_SIZE - p_back->len();
			if(num > left) { num = left; }
			memcpy(p_back->str() + p_back->len(), str+pos, num);
			p_back->len() += num;
			pos += num;
		}
	}
	
	void clear() {
		p_front->len() = 0;
		chunk_t*& next = p_front->next_chunk();
		if(next) {
			next->free_all();
			next = 0;
		}
		p_back = p_front;
	}
	
	int size() const {
		int count = 0;
		chunk_t* chunk = p_front;
		while(chunk) {
			count += chunk->len();
			chunk = chunk->next_chunk();
		}
		return count;
	}
	
	bool empty() const {
		return p_back == p_front && p_front->len() == 0;
	}
	
	chunk_t* front() const {
		return p_front;
	}
	
	chunk_t* back() const {
		return p_back;
	}
	
private:
	chunk_t* p_front = 0;
	chunk_t* p_back = 0;
	
};


class StringOutput {
public:
	virtual ~StringOutput() {}
	virtual void write(const String& str) = 0;
};

class StringWriter {
public:
	StringWriter(StringOutput* func) : func(func) {}
	StringWriter(const StringWriter& other) : func(other.func) {}
	~StringWriter() {
		if(func) {
			func->write(out);
		}
	}
private:
	StringOutput* func;
public:
	String out;
};



} // vnl


#endif /* INCLUDE_PHY_STRING_H_ */
