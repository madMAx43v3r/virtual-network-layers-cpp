/*
 * TypeInput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_TYPEINPUT_H_
#define INCLUDE_IO_TYPEINPUT_H_

#include <vnl/io/ByteInput.h>
#include <vnl/io/TypeStream.h>


namespace vnl { namespace io {

template<typename TStream>
class TypeInput : protected ByteInput<TStream> {
public:
	typedef ByteInput<TStream> Base;
	
	TypeInput(TStream& stream) : ByteInput<TStream>(stream) {}
	
	int getEntry(uint32_t& size) {
		int8_t c;
		Base::getChar(tmp);
		size = c >> 4;
		if(size == 0xF) {
			int32_t tmp = 0;
			Base::getInt(tmp);
			size = tmp;
		}
		return c & 0xF;
	}
	
	void getHash(uint32_t& hash) {
		int32_t tmp = 0;
		Base::getInt(tmp);
		hash = tmp;
	}
	
	template<typename T>
	void getInteger(T& value) {
		uint32_t size = 0;
		int id = getEntry(size);
		if(id == VNL_IO_INTEGER) {
			switch(size) {
				case VNL_IO_BYTE:  { int8_t tmp = 0; Base::getChar(tmp); value = tmp; break; }
				case VNL_IO_WORD:  { int16_t tmp = 0; Base::getShort(tmp); value = tmp; break; }
				case VNL_IO_DWORD: { int32_t tmp = 0; Base::getInt(tmp); value = tmp; break; }
				case VNL_IO_QWORD: { int64_t tmp = 0; Base::getLong(tmp); value = tmp; break; }
				default: skip_bytes(size);
			}
		} else {
			skip(id, size);
		}
	}
	
	template<typename T>
	void getReal(T& value) {
		uint32_t size = 0;
		int id = getEntry(size);
		if(id == VNL_IO_REAL) {
			switch(size) {
				case VNL_IO_DWORD: { float tmp = 0; Base::getFloat(tmp); value = tmp; break; }
				case VNL_IO_QWORD: { double tmp = 0; Base::getDouble(tmp); value = tmp; break; }
				default: skip_bytes(size);
			}
		} else {
			skip(id, size);
		}
	}
	
	template<typename T>
	void getArray(uint32_t dim, T* data, uint32_t size);
	
	void getBinary(Page* buf, uint32_t size) {
		Base::getBinary(buf, size);
	}
	
	void getString(vnl::String& str, uint32_t size) {
		Base::getString(str, size);
	}
	
	void skip() {
		uint32_t size = 0;
		int id = getEntry(size);
		skip(id, size);
	}
	
	void skip(int id, uint32_t size) {
		uint32_t hash = 0;
		switch(id) {
			case VNL_IO_NULL: break;
			case VNL_IO_INTEGER:
			case VNL_IO_REAL:
			case VNL_IO_BINARY:
			case VNL_IO_STRING:
				skip_bytes(size);
				break;
			case VNL_IO_CALL:
			case VNL_IO_CONST_CALL:
				getHash(hash);
				skip_call(size);
				break;
			case VNL_IO_ARRAY:
				getArray<void>(0, 0, size);
				break;
			case VNL_IO_STRUCT:
				skip_struct(size);
				break;
			case VNL_IO_CLASS:
				getHash(hash);
				skip_struct(size);
				break;
			case VNL_IO_INTERFACE:
				getHash(hash);
				skip_interface();
				break;
			default:
				Base::err = true;
		}
	}
	
protected:
	void skip_bytes(uint32_t size) {
		char buf[1024];
		while(size > 0 && !Base::error()) {
			uint32_t num = std::min(size, (uint32_t)sizeof(buf));
			in.read(buf, num);
			size -= num;
		}
	}
	
	void skip_struct(uint32_t size) {
		for(uint32_t i = 0; i < size && !Base::error(); ++i) {
			getHash(hash);
			skip();
		}
	}
	
	void skip_interface() {
		while(!Base::error()) {
			uint32_t size = 0;
			int id = getEntry(size);
			if(id == VNL_IO_INTERFACE && size == VNL_IO_END) {
				break;
			}
			skip(id, size);
		}
	}
	
	void skip_call(uint32_t size) {
		for(uint32_t i = 0; i < size && !Base::error(); ++i) {
			skip();
		}
	}
	
};


template<typename TStream>
template<typename T>
void TypeInput<TStream>::getArray(uint32_t dim, T* data, uint32_t size) {
	uint32_t org = size;
	uint32_t num = std::min(size, dim);
	int id = getEntry(size);
	if(id == VNL_IO_INTEGER) {
		switch(size) {
			case VNL_IO_BYTE: { int8_t tmp; for(uint32_t i = 0; i < num && !Base::error(); ++i) { Base::getChar(tmp); data[i] = tmp; } break; }
			case VNL_IO_WORD: { int16_t tmp; for(uint32_t i = 0; i < num && !Base::error(); ++i) { Base::getShort(tmp); data[i] = tmp; } break; }
			case VNL_IO_DWORD: { int32_t tmp; for(uint32_t i = 0; i < num && !Base::error(); ++i) { Base::getInt(tmp); data[i] = tmp; } break; }
			case VNL_IO_QWORD: { int64_t tmp; for(uint32_t i = 0; i < num && !Base::error(); ++i) { Base::getLong(tmp); data[i] = tmp; } break; }
			default: Base::err = true;
		}
	} else {
		num = 0;
	}
	for(uint32_t i = num; i < org; ++i) {
		skip(id, size);
	}
}

template<typename TStream>
template<>
void TypeInput<TStream>::getArray<int8_t>(uint32_t dim, int8_t* data, uint32_t size) {
	uint32_t org = size;
	uint32_t num = std::min(size, dim);
	int id = getEntry(size);
	if(id == VNL_IO_INTEGER) {
		switch(size) {
			case VNL_IO_BYTE: { Base::read(data, num); break; }
			case VNL_IO_WORD: { int16_t tmp; for(uint32_t i = 0; i < num && !Base::error(); ++i) { Base::getShort(tmp); data[i] = tmp; } break; }
			case VNL_IO_DWORD: { int32_t tmp; for(uint32_t i = 0; i < num && !Base::error(); ++i) { Base::getInt(tmp); data[i] = tmp; } break; }
			case VNL_IO_QWORD: { int64_t tmp; for(uint32_t i = 0; i < num && !Base::error(); ++i) { Base::getLong(tmp); data[i] = tmp; } break; }
			default: Base::err = true;
		}
	} else {
		num = 0;
	}
	for(uint32_t i = num; i < org; ++i) {
		skip(id, size);
	}
}

template<typename TStream>
template<>
void TypeInput<TStream>::getArray<float>(uint32_t dim, float* data, uint32_t size) {
	uint32_t org = size;
	uint32_t num = std::min(size, dim);
	int id = getEntry(size);
	if(id == VNL_IO_REAL) {
		switch(size) {
			case VNL_IO_DWORD: { float tmp; for(uint32_t i = 0; i < num && !Base::error(); ++i) { Base::getFloat(tmp); data[i] = tmp; } break; }
			case VNL_IO_QWORD: { double tmp; for(uint32_t i = 0; i < num && !Base::error(); ++i) { Base::getDouble(tmp); data[i] = tmp; } break; }
			default: Base::err = true;
		}
	} else {
		num = 0;
	}
	for(uint32_t i = num; i < org; ++i) {
		skip(id, size);
	}
}

template<typename TStream>
template<>
void TypeInput<TStream>::getArray<double>(uint32_t dim, double* data, uint32_t size) {
	uint32_t org = size;
	uint32_t num = std::min(size, dim);
	int id = getEntry(size);
	if(id == VNL_IO_REAL) {
		switch(size) {
			case VNL_IO_DWORD: { float tmp; for(uint32_t i = 0; i < num && !Base::error(); ++i) { Base::getFloat(tmp); data[i] = tmp; } break; }
			case VNL_IO_QWORD: { double tmp; for(uint32_t i = 0; i < num && !Base::error(); ++i) { Base::getDouble(tmp); data[i] = tmp; } break; }
			default: Base::err = true;
		}
	} else {
		num = 0;
	}
	for(uint32_t i = num; i < org; ++i) {
		skip(id, size);
	}
}




}}

#endif /* INCLUDE_IO_TYPEINPUT_H_ */
