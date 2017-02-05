/*
 * Stream.h
 *
 *  Created on: Jun 30, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_IO_STREAM_H_
#define INCLUDE_VNL_IO_STREAM_H_

#include <vnl/io/Error.h>


namespace vnl { namespace io {

class InputStream {
public:
	virtual ~InputStream() {}
	
	int error() const {
		return err;
	}
	
	void set_error(int err_) {
		err = err_;
#ifdef VNL_IO_DEBUG
		assert(err == VNL_SUCCESS || err == VNL_IO_EOF);
#endif
	}
	
	void reset() {
		err = VNL_SUCCESS;
	}
	
	// read some bytes
	virtual int read(void* dst, int len) = 0;
	
protected:
	int err = 0;
	
};


class OutputStream {
public:
	virtual ~OutputStream() {}
	
	int error() const {
		return err;
	}
	
	void set_error(int err_) {
		err = err_;
#ifdef VNL_IO_DEBUG
		assert(err == VNL_SUCCESS || err == VNL_IO_EOF);
#endif
	}
	
	void reset() {
		err = VNL_SUCCESS;
	}
	
	// write all bytes
	virtual bool write(const void* src, int len) = 0;
	
protected:
	int err = 0;
	
};


}}

#endif /* INCLUDE_VNL_IO_STREAM_H_ */
