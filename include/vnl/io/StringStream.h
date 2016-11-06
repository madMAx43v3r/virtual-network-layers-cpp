/*
 * StringStream.h
 *
 *  Created on: Sep 13, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_STRINGSTREAM_H_
#define INCLUDE_VNL_STRINGSTREAM_H_

#include <vnl/io/Stream.h>


namespace vnl { namespace io {

class StringStream : public InputStream, public OutputStream {
public:
	StringStream(String& str) : in(&str), out(&str) {}
	StringStream(const String& str) : in(&str), out(0) {}
	
	virtual int read(void* dst, int len) {
		// TODO
		return 0;
	}
	
	virtual bool write(const void* src, int len) {
		if(!out) {
			return false;
		}
		// TODO
		return false;
	}
	
private:
	const String* in;
	String* out;
	
};



}}

#endif /* INCLUDE_VNL_STRINGSTREAM_H_ */
