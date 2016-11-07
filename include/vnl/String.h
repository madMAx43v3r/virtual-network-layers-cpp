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
	char buf[128];
	int len = 0;
};

class str : public ToString {
public:
	str(const char src[]) {
		len = strlen(src);
		if(len > sizeof(buf)) {
			len = sizeof(buf);
		}
		strncpy(buf, src, len);
	}
};

static str endl("\n");

class dec : public ToString {
public:
	dec(int32_t i)  { len = snprintf(buf, sizeof(buf), "%d", i); }
	dec(uint32_t i) { len = snprintf(buf, sizeof(buf), "%u", i); }
	dec(int64_t i)  { len = snprintf(buf, sizeof(buf), "%ld", i); }
	dec(uint64_t i) { len = snprintf(buf, sizeof(buf), "%lu", i); }
};

class hex : public ToString {
public:
	hex(int32_t i)  { len = snprintf(buf, sizeof(buf), "0x%x", i); }
	hex(uint32_t i) { len = snprintf(buf, sizeof(buf), "0x%x", i); }
	hex(int64_t i)  { len = snprintf(buf, sizeof(buf), "0x%lx", i); }
	hex(uint64_t i) { len = snprintf(buf, sizeof(buf), "0x%lx", i); }
};

class def : public ToString {
public:
	def(float f, int precision = 6)  { len = snprintf(buf, sizeof(buf), "%.*f", precision, f); }
	def(double f, int precision = 6) { len = snprintf(buf, sizeof(buf), "%.*f", precision, f); }
};

class sci : public ToString {
public:
	sci(float f, int precision = 6)  { len = snprintf(buf, sizeof(buf), "%.*e", precision, f); }
	sci(double f, int precision = 6) { len = snprintf(buf, sizeof(buf), "%.*e", precision, f); }
};

class fix : public ToString {
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
	
	String(String::const_iterator itbegin, String::const_iterator itend) : Array(itbegin, itend) {
		// nothing to do
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
	
	String trim() const {
		String::const_iterator first = end();
		String::const_iterator last = begin();
		for(String::const_iterator it=begin(); it!=end(); ++it) {
			switch(*it) {
				case ' ':
				case '\t':
				case '\n':
				case '\v':
				case '\f':
				case '\r':
					break;
				default:
					if(first == end()) {
						first = it;
					}
					last = it;
					last++;
			}
		}
		return String(first, last);
	}

	ssize_t find(char c, size_t pos=0, ssize_t n=-1) const {
		size_t i = 0;
		for(String::const_iterator it=begin(); it!=end(); ++it) {
			if(i >= pos && (n < 0 || i < pos+n) && *it == c) {
				return i;
			}
			i++;
		}
		return -1;
	}

	String substr(size_t pos=0, ssize_t len=-1) const {
		String::const_iterator itbeg = end();
		String::const_iterator itend = end();
		int i=0;
		for(String::const_iterator it=begin(); it!=end(); ++it) {
			if(itbeg == end() && i >= pos) {
				itbeg = it;
			}
			if(len >= 0 && i >= pos+len) {
				itend = it;
				break;
			}
		}
		return String(itbeg, itend);
	}

	Array<String> tokenize(char tok, bool trim=true) const {
		Array<String> list;
		String::const_iterator it0 = begin();
		for(String::const_iterator it=begin(); it!=end(); ++it) {
			if(*it == tok) {
				list.push_back(trim ? String(it0, it).trim() : String(it0, it));
				it0 = it;
				++it0;
			}
		}
		list.push_back(trim ? String(it0, end()).trim() : String(it0, end()));
		return list;
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
