/*
 * String.h
 *
 *  Created on: May 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_STRING_H_
#define INCLUDE_PHY_STRING_H_

#include <ostream>
#include <streambuf>
#include <string>
#include <sstream>

#include "phy/List.h"


namespace vnl { namespace phy {

class String : public std::ostream, private std::streambuf {
public:
	String(Region* mem) : std::iostream(this), list(mem) {}
	
	~String() {
		if(buf) {
			buf->free();
		}
	}
	
	String(const String& other) = delete;
	
	String& operator=(const String& other) {
		list.clear();
		(*this) << other;
		return *this;
	}
	
	std::ostream& operator<<(String& other) {
		other.sync();
		for(const str_t& str : other.list) {
			std::ostream::write(str.ptr, str.len);
		}
		return *this;
	}
	
	std::string to_str() {
		sync();
		std::ostringstream ss;
		for(const str_t& str : list) {
			ss.write(str.ptr, str.len);
		}
		return ss.str();
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
			str.len = Page::size;
			str.ptr = list.mem->alloc(str.len);
			memcpy(str.ptr, buf, Page::size);
			list.push_back(str);
		} else {
			buf = Page::alloc();
		}
		setp(buf->mem, buf->mem + Page::size);
		sputc(c);
		return c;
	}
	
	virtual int sync() override {
		if(buf) {
			str_t str;
			str.len = pptr() - pbase();
			str.ptr = list.mem->alloc(str.len);
			memcpy(str.ptr, buf, str.len);
			list.push_back(str);
			setp(0, 0);
			buf->free();
			buf = 0;
		}
		return 0;
	}
	
private:
	List<str_t, 2> list;
	Page* buf = 0;
	
};





}}

#endif /* INCLUDE_PHY_STRING_H_ */
