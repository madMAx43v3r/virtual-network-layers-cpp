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
	
	virtual int read(void* buf, int len) = 0;
	virtual bool write(const void* buf, int len) = 0;
	
};

}}

#endif /* INCLUDE_IO_STREAM_H_ */
