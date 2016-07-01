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


namespace vnl { namespace io {

class TypeOutput : public ByteOutput {
public:
	TypeOutput(OutputStream* stream) : ByteOutput(stream) {}
	
	void putEntry(int id, int size) {
		int8_t c = size >= 0xF ? 0xF : size;
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
	
	void putBool(bool value) {
		putEntry(VNL_IO_BOOL, value ? VNL_IO_TRUE : VNL_IO_FALSE);
	}
	
	void putChar(int8_t value) {
		putEntry(VNL_IO_INTEGER, VNL_IO_BYTE);
		writeChar(value);
	}
	
	void putShort(int16_t value) {
		if(value > -128 && value < 128) {
			putChar(value);
		} else {
			putEntry(VNL_IO_INTEGER, VNL_IO_WORD);
			writeShort(value);
		}
	}
	
	void putInt(int32_t value) {
		if(value > -32768 && value < 32768) {
			putShort(value);
		} else {
			putEntry(VNL_IO_INTEGER, VNL_IO_DWORD);
			writeInt(value);
		}
	}
	
	void putLong(int64_t value) {
		if(value > -2147483648 && value < 2147483648) {
			putInt(value);
		} else {
			putEntry(VNL_IO_INTEGER, VNL_IO_QWORD);
			writeLong(value);
		}
	}
	
	void putFloat(float value) {
		putEntry(VNL_IO_REAL, VNL_IO_DWORD);
		writeFloat(value);
	}
	
	void putDouble(double value) {
		putEntry(VNL_IO_REAL, VNL_IO_QWORD);
		writeDouble(value);
	}
	
	template<typename T>
	void putArray(const T* data, int dim) {
		putEntry(VNL_IO_ARRAY, 0);
		putEntry(VNL_IO_NULL, 0);
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


template<>
void TypeOutput::putArray<int8_t>(const int8_t* data, int dim) {
	putEntry(VNL_IO_ARRAY, dim);
	putEntry(VNL_IO_INTEGER, VNL_IO_BYTE);
	write(data, dim);
}

template<>
void TypeOutput::putArray<int16_t>(const int16_t* data, int dim) {
	putEntry(VNL_IO_ARRAY, dim);
	putEntry(VNL_IO_INTEGER, VNL_IO_WORD);
	for(int i = 0; i < dim; ++i) {
		writeShort(data[i]);
	}
}

template<>
void TypeOutput::putArray<int32_t>(const int32_t* data, int dim) {
	putEntry(VNL_IO_ARRAY, dim);
	putEntry(VNL_IO_INTEGER, VNL_IO_DWORD);
	for(int i = 0; i < dim; ++i) {
		writeInt(data[i]);
	}
}

template<>
void TypeOutput::putArray<int64_t>(const int64_t* data, int dim) {
	putEntry(VNL_IO_ARRAY, dim);
	putEntry(VNL_IO_INTEGER, VNL_IO_QWORD);
	for(int i = 0; i < dim; ++i) {
		writeLong(data[i]);
	}
}

template<>
void TypeOutput::putArray<float>(const float* data, int dim) {
	putEntry(VNL_IO_ARRAY, dim);
	putEntry(VNL_IO_REAL, VNL_IO_DWORD);
	for(int i = 0; i < dim; ++i) {
		writeFloat(data[i]);
	}
}

template<>
void TypeOutput::putArray<double>(const double* data, int dim) {
	putEntry(VNL_IO_ARRAY, dim);
	putEntry(VNL_IO_REAL, VNL_IO_QWORD);
	for(int i = 0; i < dim; ++i) {
		writeDouble(data[i]);
	}
}



}}

#endif /* INCLUDE_IO_TYPEOUTPUT_H_ */
