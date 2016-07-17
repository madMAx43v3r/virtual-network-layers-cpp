/*
 * TypeOutput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_TYPEOUTPUT_H_
#define INCLUDE_IO_TYPEOUTPUT_H_

#include <vnl/io/ByteOutput.h>
#include <vnl/io/TypeStream.h>
#include <vnl/Vector.h>


namespace vnl { namespace io {

class TypeOutput : public ByteOutput {
public:
	TypeOutput(OutputStream* stream) : ByteOutput(stream) {}
	
	void putEntry(int id, int size) {
		uint8_t c = size >= 0xF ? 0xF : size;
		c <<= 4;
		c |= id & 0xF;
		writeChar(c);
		if(size >= 0xF) {
			writeInt(size);
		}
	}
	
	void putHash(uint32_t hash) {
		writeInt(hash);
	}
	
	void putNull() {
		putEntry(VNL_IO_NULL, VNL_IO_NULL);
	}
	
	void putValue(bool value) {
		putEntry(VNL_IO_BOOL, value ? VNL_IO_TRUE : VNL_IO_FALSE);
	}
	
	void putValue(int8_t value) {
		putEntry(VNL_IO_INTEGER, VNL_IO_BYTE);
		writeChar(value);
	}
	
	void putValue(int16_t value) {
		if(value > -128 && value < 128) {
			putValue((int8_t)value);
		} else {
			putEntry(VNL_IO_INTEGER, VNL_IO_WORD);
			writeShort(value);
		}
	}
	
	void putValue(int32_t value) {
		if(value > -32768 && value < 32768) {
			putValue((int16_t)value);
		} else {
			putEntry(VNL_IO_INTEGER, VNL_IO_DWORD);
			writeInt(value);
		}
	}
	
	void putValue(int64_t value) {
		if(value > -2147483648 && value < 2147483648) {
			putValue((int32_t)value);
		} else {
			putEntry(VNL_IO_INTEGER, VNL_IO_QWORD);
			writeLong(value);
		}
	}
	
	void putValue(float value) {
		putEntry(VNL_IO_REAL, VNL_IO_DWORD);
		writeFloat(value);
	}
	
	void putValue(double value) {
		putEntry(VNL_IO_REAL, VNL_IO_QWORD);
		writeDouble(value);
	}
	
	template<typename T, int N>
	void putArray(const vnl::Vector<T, N>& vec) {
		putArray(&vec[0], N);
	}
	
	template<typename T>
	void putArray(const T* data, int dim) {
		putEntry(VNL_IO_ARRAY, dim);
		for(int i = 0; i < dim; ++i) {
			putValue(data[i]);
		}
	}
	
	void putBinary(vnl::Page* buf, int size) {
		putEntry(VNL_IO_BINARY, size);
		writeBinary(buf, size);
	}
	
	void putString(const vnl::String& str) {
		putEntry(VNL_IO_STRING, str.size());
		writeString(str);
	}
	
};




}}

#endif /* INCLUDE_IO_TYPEOUTPUT_H_ */
