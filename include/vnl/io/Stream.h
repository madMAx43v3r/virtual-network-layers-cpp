/*
 * Stream.h
 *
 *  Created on: Jun 30, 2016
 *      Author: mad
 */

#ifndef STREAM_H_
#define STREAM_H_


namespace vnl { namespace io {

class Stream {
public:
	virtual ~Stream() {}
	
};


class InputStream : public Stream {
public:
	
	// read some bytes
	virtual int read(void* dst, int len) = 0;
	
};


class OutputStream : public Stream {
public:
	
	// write all bytes
	virtual bool write(const void* src, int len) = 0;
	
};


}}

#endif /* STREAM_H_ */
