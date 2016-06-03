/*
 * Stream.h
 *
 *  Created on: Jan 8, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_STREAM_H_
#define INCLUDE_IO_STREAM_H_

namespace vnl { namespace io {

class Stream {
public:
	virtual ~Stream() {}
	
	virtual bool read(void* dst, int len) = 0;
	virtual bool write(const void* src, int len) = 0;
	
	virtual bool flush() = 0;
	
};


}}

#endif /* INCLUDE_IO_STREAM_H_ */