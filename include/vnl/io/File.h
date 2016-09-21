/*
 * File.h
 *
 *  Created on: Sep 14, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_VNL_IO_FILE_H_
#define INCLUDE_VNL_IO_FILE_H_

#include <stdio.h>

#include <vnl/io/Stream.h>


namespace vnl { namespace io {

class File : public InputStream, public OutputStream {
public:
	File() : fd(0) {}
	
	File(FILE* fd) : fd(fd) {}
	
	operator FILE*() const {
		return fd;
	}
	
	bool good() const {
		return fd != 0;
	}
	
	File& operator=(FILE* file) {
		fd = file;
		return *this;
	}
	
	virtual int read(void* dst, int len) {
		int res = ::fread(dst, 1, len, fd);
		if(res <= 0) {
			InputStream::set_error(VNL_IO_EOF);
		}
		return res;
	}
	
	virtual bool write(const void* src, int len) {
		while(len > 0) {
			int res = ::fwrite(src, 1, len, fd);
			if(res > 0) {
				len -= res;
				src = (char*)src + res;
			} else {
				OutputStream::set_error(VNL_IO_EOF);
				return false;
			}
		}
		return true;
	}
	
	void close() {
		::fclose(fd);
		fd = 0;
	}
	
private:
	FILE* fd;
	
};



}}

#endif /* INCLUDE_VNL_IO_FILE_H_ */
