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

class String;

template<int N>
class FixedString {
public:
	char buf[N];
	int len = 0;
	FixedString() {}
	FixedString(const String& str) {
		*this = str;
	}
	FixedString(const char* src) {
		*this = src;
	}
	FixedString& operator=(const String& str);
	FixedString& operator=(const char* src) {
		len = strlen(src);
		if(len > N) { len = N; }
		strncpy(buf, src, len);
		buf[N-1] = 0;
		return *this;
	}
};

static FixedString<128> endl("\n");

inline int32_t atol(const vnl::String& str) {
	FixedString<128> tmp = str;
	return ::strtol(tmp.buf, 0, 10);
}

inline int64_t atoll(const vnl::String& str) {
	FixedString<128> tmp = str;
	return ::strtoll(tmp.buf, 0, 10);
}

inline uint32_t atoul(const vnl::String& str) {
	FixedString<128> tmp = str;
	return ::strtoul(tmp.buf, 0, 10);
}

inline uint64_t atoull(const vnl::String& str) {
	FixedString<128> tmp = str;
	return ::strtoull(tmp.buf, 0, 10);
}

inline double atof(const vnl::String& str) {
	FixedString<128> tmp = str;
	return ::atof(tmp.buf);
}

class dec : public FixedString<128> {
public:
	dec(int32_t i)  { len = snprintf(buf, sizeof(buf), "%d", i); }
	dec(uint32_t i) { len = snprintf(buf, sizeof(buf), "%u", i); }
	dec(int64_t i)  { len = snprintf(buf, sizeof(buf), "%ld", i); }
	dec(uint64_t i) { len = snprintf(buf, sizeof(buf), "%lu", i); }
};

class hex : public FixedString<128> {
public:
	hex(int32_t i)  { len = snprintf(buf, sizeof(buf), "0x%x", i); }
	hex(uint32_t i) { len = snprintf(buf, sizeof(buf), "0x%x", i); }
	hex(int64_t i)  { len = snprintf(buf, sizeof(buf), "0x%lx", i); }
	hex(uint64_t i) { len = snprintf(buf, sizeof(buf), "0x%lx", i); }
};

class def : public FixedString<128> {
public:
	def(float f, int precision = 6)  { len = snprintf(buf, sizeof(buf), "%.*f", precision, f); }
	def(double f, int precision = 6) { len = snprintf(buf, sizeof(buf), "%.*f", precision, f); }
};

class sci : public FixedString<128> {
public:
	sci(float f, int precision = 6)  { len = snprintf(buf, sizeof(buf), "%.*e", precision, f); }
	sci(double f, int precision = 6) { len = snprintf(buf, sizeof(buf), "%.*e", precision, f); }
};

class fix : public FixedString<128> {
public:
	fix(float f, int precision = 6)  { len = snprintf(buf, sizeof(buf), "%0*f", precision, f); }
	fix(double f, int precision = 6) { len = snprintf(buf, sizeof(buf), "%0*f", precision, f); }
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
		assign(str);
	}
	
	bool operator!=(const String& other) const {
		return !(*this == other);
	}
	
	bool operator==(const String& other) const {
		return ((const Array&)(*this)) == ((const Array&)other);
	}
	
	bool operator<(const String& other) const {
		const_iterator cmp = other.begin();
		for(const_iterator it = begin(); it != end(); ++it, ++cmp) {
			if(cmp == other.end()) {
				return false;
			}
			if(*it < *cmp) {
				return true;
			}
		}
		return cmp != other.end();
	}
	
	bool operator<=(const String& other) const {
		return *this < other || *this == other;
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
			assign(str);
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
		if(str) {
			write(str, strlen(str));
		}
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
	
	template<int N>
	String& operator<<(const FixedString<N>& str) {
		write(str.buf, str.len);
		return *this;
	}
	
	std::string to_string() const {
		std::string res;
		for(const_iterator it = begin(); it != end() && *it != 0; ++it) {
			res += *it;
		}
		return res;
	}
	
	int to_string(char* str, int len) const {
		memset(str, 0, len);
		int i = 0;
		for(const_iterator it = begin(); it != end() && *it != 0; ++it) {
			if(i >= len-1) {
				break;
			}
			str[i] = *it;
			i++;
		}
		return i;
	}
	
	void assign(const String& str) {
		clear();
		Block* src = str.p_front;
		while(src) {
			Block* next = Block::alloc();
			if(p_back) {
				p_back->next = next;
			} else {
				p_front = next;
			}
			p_back = next;
			::memcpy(p_back->mem, src->mem, src->next ? Block::size : str.pos);
			src = src->next;
		}
		pos = str.pos;
		count = str.count;
	}
	
	void write(const char* str, int len) {
		check();
		count += len;
		while(len > 0) {
			if(pos >= Block::size) {
				extend();
			}
			int n = Block::size - pos;
			if(n > len) {
				n = len;
			}
			::memcpy(p_back->mem + pos, str, n);
			pos += n;
			str += n;
			len -= n;
		}
	}
	
	friend std::ostream& operator<<(std::ostream& stream, const String& str) {
		for(const_iterator it = str.begin(); it != str.end(); ++it) {
			stream.put(*it);
		}
		return stream;
	}
	
};

template<int N>
FixedString<N>& FixedString<N>::operator=(const String& str) {
	len = str.to_string(buf, N);
	return *this;
}


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
