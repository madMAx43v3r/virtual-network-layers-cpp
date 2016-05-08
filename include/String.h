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

#include "List.h"


namespace vnl {

class String : public std::ostream, private std::streambuf {
public:
	String(phy::Region* mem) : std::ostream(this), list(mem) {}
	
	~String() {
		if(buf) {
			buf->free();
		}
	}
	
	String(const String& other) = delete;
	
	String& operator=(String& other) {
		list.clear();
		(*this) << other;
		return *this;
	}
	
	String& operator<<(String& other) {
		other.sync();
		for(const str_t& str : other.list) {
			std::ostream::write(str.ptr, str.len);
		}
		return *this;
	}
	
	std::string to_string() {
		sync();
		std::ostringstream ss;
		for(const str_t& str : list) {
			ss.write(str.ptr, str.len);
		}
		return ss.str();
	}
	
	friend std::ostream& operator<<(std::ostream& stream, String& other) {
		other.sync();
		for(const String::str_t& str : other.list) {
			stream.write(str.ptr, str.len);
		}
		return stream;
	}
	
	void clear() {
		list.clear();
		if(buf) {
			buf->free();
		}
	}
	
protected:
	struct str_t {
		char* ptr = 0;
		size_t len = 0;
	};
	
	virtual int overflow(int c = std::char_traits<char>::eof()) override {
		if(buf) {
			str_t str;
			str.len = phy::Page::size;
			str.ptr = (char*)list.mem->alloc(str.len);
			memcpy(str.ptr, buf->mem, phy::Page::size);
			list.push_back(str);
		} else {
			buf = phy::Page::alloc();
		}
		setp(buf->mem, buf->mem + phy::Page::size);
		sputc(c);
		return c;
	}
	
	virtual int sync() override {
		if(buf) {
			str_t str;
			str.len = pptr() - pbase();
			str.ptr = (char*)list.mem->alloc(str.len);
			memcpy(str.ptr, buf->mem, str.len);
			list.push_back(str);
			setp(0, 0);
			buf->free();
			buf = 0;
		}
		return 0;
	}
	
private:
	List<str_t, 2> list;
	phy::Page* buf = 0;
	
};


} // vnl




#endif /* INCLUDE_PHY_STRING_H_ */
