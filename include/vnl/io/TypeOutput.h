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

template<typename TStream>
class TypeOutput : protected ByteOutput<TStream> {
public:
	typedef ByteOutput<TStream> Base;
	
	TypeOutput(TStream& stream) : ByteOutput<TStream>(stream) {}
	
	void putEntry(int id, uint32_t size) {
		int8_t c = size >= 0xF ? 0xF : size;
		c <<= 4;
		c |= id & 0xF;
		Base::writeChar(c);
		if(size >= 0xF) {
			Base::writeInt(size);
		}
	}
	
	void putHash(uint32_t hash) {
		Base::writeInt(hash);
	}
	
	void putNull() {
		putEntry(VNL_IO_NULL, VNL_IO_NULL);
	}
	
	void putChar(int8_t value) {
		putEntry(VNL_IO_INTEGER, VNL_IO_BYTE);
		Base::writeChar(value);
	}
	
	void putShort(int16_t value) {
		if(value > -128 && value < 128) {
			putChar(value);
		} else {
			putEntry(VNL_IO_INTEGER, VNL_IO_WORD);
			Base::writeShort(value);
		}
	}
	
	void putInt(int32_t value) {
		if(value > -32768 && value < 32768) {
			putShort(value);
		} else {
			putEntry(VNL_IO_INTEGER, VNL_IO_DWORD);
			Base::writeInt(value);
		}
	}
	
	void putLong(int64_t value) {
		if(value > -2147483648 && value < 2147483648) {
			putInt(value);
		} else {
			putEntry(VNL_IO_INTEGER, VNL_IO_QWORD);
			Base::writeLong(value);
		}
	}
	
	void putFloat(float value) {
		putEntry(VNL_IO_REAL, VNL_IO_DWORD);
		Base::writeFloat(value);
	}
	
	void putDouble(double value) {
		putEntry(VNL_IO_REAL, VNL_IO_QWORD);
		Base::writeDouble(value);
	}
	
	template<typename T>
	void putArray(uint32_t dim, const T* data) {
		putEntry(VNL_IO_ARRAY, 0);
		putEntry(VNL_IO_NULL, 0);
	}
	
	void putBinary(vnl::Page* buf, uint32_t size) {
		putEntry(VNL_IO_BINARY, size);
		Base::writeBinary(buf, size);
	}
	
	void putString(const vnl::String& str) {
		uint32_t size = str.size();
		putEntry(VNL_IO_STRING, size);
		Base::writeString(str, size);
	}
	
};


template<typename TStream>
template<>
void TypeOutput<TStream>::putArray<int8_t>(uint32_t dim, const int8_t* data) {
	putEntry(VNL_IO_ARRAY, dim);
	putEntry(VNL_IO_INTEGER, VNL_IO_BYTE);
	Base::write(data, dim);
}

template<typename TStream>
template<>
void TypeOutput<TStream>::putArray<int16_t>(uint32_t dim, const int16_t* data) {
	putEntry(VNL_IO_ARRAY, dim);
	putEntry(VNL_IO_INTEGER, VNL_IO_WORD);
	for(uint32_t i = 0; i < dim; ++i) {
		Base::writeShort(data[i]);
	}
}

template<typename TStream>
template<>
void TypeOutput<TStream>::putArray<int32_t>(uint32_t dim, const int32_t* data) {
	putEntry(VNL_IO_ARRAY, dim);
	putEntry(VNL_IO_INTEGER, VNL_IO_DWORD);
	for(uint32_t i = 0; i < dim; ++i) {
		Base::writeInt(data[i]);
	}
}

template<typename TStream>
template<>
void TypeOutput<TStream>::putArray<int64_t>(uint32_t dim, const int64_t* data) {
	putEntry(VNL_IO_ARRAY, dim);
	putEntry(VNL_IO_INTEGER, VNL_IO_QWORD);
	for(uint32_t i = 0; i < dim; ++i) {
		Base::writeLong(data[i]);
	}
}

template<typename TStream>
template<>
void TypeOutput<TStream>::putArray<float>(uint32_t dim, const float* data) {
	putEntry(VNL_IO_ARRAY, dim);
	putEntry(VNL_IO_REAL, VNL_IO_DWORD);
	for(uint32_t i = 0; i < dim; ++i) {
		Base::writeFloat(data[i]);
	}
}

template<typename TStream>
template<>
void TypeOutput<TStream>::putArray<double>(uint32_t dim, const double* data) {
	putEntry(VNL_IO_ARRAY, dim);
	putEntry(VNL_IO_REAL, VNL_IO_QWORD);
	for(uint32_t i = 0; i < dim; ++i) {
		Base::writeDouble(data[i]);
	}
}



}}

#endif /* INCLUDE_IO_TYPEOUTPUT_H_ */
