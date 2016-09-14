/*
 * File.h
 *
 *  Created on: Sep 14, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_VNL_IO_FILE_H_
#define INCLUDE_VNL_IO_FILE_H_

#include <unistd.h>

#include <vnl/io/Stream.h>


namespace vnl { namespace io {

class File : public InputStream, public OutputStream {
public:
	int fd;
	
	File() : fd(-1) {}
	
	File(int fd) : fd(fd) {}
	
	virtual int read(void* dst, int len) {
		int res = ::read(fd, dst, len);
		if(res <= 0) {
			InputStream::set_error(VNL_IO_CLOSED);
		}
		return res;
	}
	
	virtual bool write(const void* src, int len) {
		while(len > 0) {
			int res = ::write(fd, src, len);
			if(res > 0) {
				len -= res;
				src = (char*)src + res;
			} else {
				OutputStream::set_error(VNL_IO_CLOSED);
				return false;
			}
		}
		return true;
	}
	
	void close() {
		::close(fd);
	}
	
};



}}

#endif /* INCLUDE_VNL_IO_FILE_H_ */
