/*
 * File.h
 *
 *  Created on: Sep 14, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_VNL_IO_FILE_H_
#define INCLUDE_VNL_IO_FILE_H_

#include <stdio.h>
#include <sys/file.h>

#include <vnl/io/Stream.h>


namespace vnl { namespace io {

class File : public InputStream, public OutputStream {
public:
	File() : file(0) {}
	
	File(FILE* fd) : file(fd) {}
	
	operator FILE*() const {
		return file;
	}
	
	operator bool() const {
		return good();
	}
	
	bool good() const {
		return file != 0;
	}
	
	int get_fd() const {
		return ::fileno(file);
	}
	
	File& operator=(FILE* file_) {
		file = file_;
		return *this;
	}
	
	virtual int read(void* dst, int len) {
		if(!file) {
			InputStream::set_error(VNL_ERROR);
			return -1;
		}
		int res = ::fread(dst, 1, len, file);
		if(res <= 0) {
			InputStream::set_error(VNL_IO_EOF);
		}
		return res;
	}
	
	virtual bool write(const void* src, int len) {
		if(!file) {
			InputStream::set_error(VNL_ERROR);
			return -1;
		}
		while(len > 0) {
			int res = ::fwrite(src, 1, len, file);
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
	
private:
	FILE* file;
	
};



}}

#endif /* INCLUDE_VNL_IO_FILE_H_ */
