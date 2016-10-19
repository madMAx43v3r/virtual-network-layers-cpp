/*
 * String.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_STRING_H_
#define INCLUDE_PHY_STRING_H_

#include <string.h>
#include <sstream>
#include <string>

#include <vnl/Array.h>


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


class String : public Array<char, Block> {
public:
	String() : Array() {}
	
	String(const char* str) : Array() {
		(*this) << str;
	}
	
	String(const std::string& str) : Array() {
		(*this) << str;
	}
	
	String(const String& str) : Array() {
		(*this) << str;
	}
	
	bool operator!=(const String& other) const {
		return !(*this == other);
	}
	
	bool operator==(const String& other) const {
		return ((const Array&)(*this)) == ((const Array&)other);
	}
	
	bool operator<(const String& other) const {
		// TODO
		assert(false);
		return false;
	}
	
	bool operator>(const String& other) const {
		// TODO
		assert(false);
		return false;
	}
	
	bool operator==(const std::string& other) const {
		return (*this) == other.c_str();
	}
	
	bool operator==(const char* other) const {
		int i = 0;
		for(const_iterator it = begin(); it != end(); ++it) {
			if(other[i] != *it || other[i] == 0) {
				return false;
			}
			i++;
		}
		return other[i] == 0;
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
		if(&str != this) {
			clear();
			*this << str;
		}
		return *this;
	}
	
	String& operator<<(const void* p) {
		return *this << vnl::hex((size_t)p);
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
		append(str);
		return *this;
	}
	
	String& operator<<(const ToString& str) {
		write(str.buf, str.len);
		return *this;
	}
	
	std::string to_string() const {
		std::string res;
		for(const_iterator it = begin(); it != end(); ++it) {
			res += *it;
		}
		return res;
	}
	
	void to_string(char* str, int len) const {
		memset(str, 0, len);
		int i = 0;
		for(const_iterator it = begin(); it != end(); ++it) {
			if(i >= len-1) {
				break;
			}
			str[i] = *it;
			i++;
		}
	}
	
	friend std::ostream& operator<<(std::ostream& stream, const String& str) {
		for(const_iterator it = str.begin(); it != str.end(); ++it) {
			stream.put(*it);
		}
		return stream;
	}
	
	void write(const char* str, int len) {
		for(int i = 0; i < len; ++i) {
			push_back(str[i]);
		}
	}
	
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
