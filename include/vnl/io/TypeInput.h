/*
 * TypeInput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_TYPEINPUT_H_
#define INCLUDE_IO_TYPEINPUT_H_

#include <vnl/io/ByteInput.h>
#include <vnl/io/TypeOutput.h>
#include <vnl/io/TypeStream.h>


namespace vnl { namespace io {

class TypeInput : public ByteInput {
public:
	TypeInput(InputStream* stream) : ByteInput(stream) {}
	
	int getEntry(int& size) {
		uint8_t c = 0;
		readChar(c);
		size = c >> 4;
		if(size == 0xF) {
			int32_t tmp = 0;
			readInt(tmp);
			size = tmp;
		}
		return c & 0xF;
	}
	
	void getHash(uint32_t& hash) {
		readInt(hash);
	}
	
	template<typename T>
	void readValue(T& value, int id, int size) {
		switch(id) {
		case VNL_IO_BOOL:
			value = size == VNL_IO_TRUE;
			break;
		case VNL_IO_INTEGER:
			switch(size) {
				case VNL_IO_BYTE:  { int8_t tmp = 0; readChar(tmp); value = tmp; break; }
				case VNL_IO_WORD:  { int16_t tmp = 0; readShort(tmp); value = tmp; break; }
				case VNL_IO_DWORD: { int32_t tmp = 0; readInt(tmp); value = tmp; break; }
				case VNL_IO_QWORD: { int64_t tmp = 0; readLong(tmp); value = tmp; break; }
				default: set_error(VNL_IO_INVALID_SIZE);
			}
			break;
		case VNL_IO_REAL:
			switch(size) {
				case VNL_IO_DWORD: { float tmp = 0; readFloat(tmp); value = tmp; break; }
				case VNL_IO_QWORD: { double tmp = 0; readDouble(tmp); value = tmp; break; }
				default: set_error(VNL_IO_INVALID_SIZE);
			}
			break;
		default:
			skip(id, size);
		}
	}
	
	template<typename T>
	void getValue(T& value) {
		int size;
		int id = getEntry(size);
		readValue(value, id, size);
	}
	
	template<typename T>
	void getArray(T* data, int dim) {
		int size;
		int id = getEntry(size);
		if(id == VNL_IO_ARRAY) {
			getArray(data, dim, size);
		} else {
			skip(id, size);
		}
	}
	
	void getBinary(vnl::Page*& buf, int& size) {
		int id = getEntry(size);
		if(id == VNL_IO_BINARY) {
			readBinary(buf, size);
		} else {
			skip(id, size);
		}
	}
	
	void getString(vnl::String& str) {
		int size;
		int id = getEntry(size);
		if(id == VNL_IO_STRING) {
			readString(str, size);
		} else {
			skip(id, size);
		}
	}
	
	void skip() {
		copy(0);
	}
	
	void skip(int id, int size) {
		copy(0, id, size);
	}
	
	void skip(int id, int size, uint32_t hash) {
		copy(0, id, size, hash);
	}
	
	void copy(TypeOutput* dst) {
		int size = 0;
		int id = copy_entry(size, dst);
		copy(dst, id, size);
	}
	
	void copy(TypeOutput* dst, int id, int size) {
		uint32_t hash = 0;
		switch(id) {
			case VNL_IO_CALL:
			case VNL_IO_CONST_CALL:
			case VNL_IO_CLASS:
			case VNL_IO_INTERFACE:
				getHash(hash);
				if(dst) {
					dst->putHash(hash);
				}
				break;
		}
		copy(dst, id, size, hash);
	}
	
	void copy(TypeOutput* dst, int id, int size, uint32_t hash) {
		switch(id) {
			case VNL_IO_NULL: break;
			case VNL_IO_BOOL: break;
			case VNL_IO_INTEGER:
			case VNL_IO_REAL:
			case VNL_IO_BINARY:
			case VNL_IO_STRING:
				copy_bytes(size, dst);
				break;
			case VNL_IO_CALL:
			case VNL_IO_CONST_CALL:
				copy_call(size, dst);
				break;
			case VNL_IO_ARRAY:
				copy_array(size, dst);
				break;
			case VNL_IO_STRUCT:
			case VNL_IO_CLASS:
				copy_struct(size, dst);
				break;
			case VNL_IO_INTERFACE:
				copy_interface(dst);
				break;
			default:
				set_error(VNL_IO_INVALID_ID);
		}
	}
	
protected:
	template<typename T>
	void getArray(T* data, int dim, int size) {
		int org = size;
		int num = std::min(size, dim);
		for(int i = 0; i < num && !error(); ++i) {
			getValue(data[i]);
		}
		for(int i = num; i < org && !error(); ++i) {
			skip();
		}
	}
	
	int copy_entry(int& size, TypeOutput* dst) {
		int id = getEntry(size);
		if(dst) {
			dst->putEntry(id, size);
		}
		return id;
	}
	
	void copy_bytes(int size, TypeOutput* dst) {
		char buf[1024];
		while(size > 0 && !error()) {
			int num = std::min(size, (int)sizeof(buf));
			read(buf, num);
			if(dst) {
				dst->write(buf, num);
			}
			size -= num;
		}
	}
	
	void copy_array(int size, TypeOutput* dst) {
		for(int i = 0; i < size && !error(); ++i) {
			copy(dst);
		}
	}
	
	void copy_struct(int size, TypeOutput* dst) {
		for(int i = 0; i < size && !error(); ++i) {
			uint32_t hash = 0;
			getHash(hash);
			if(dst) {
				dst->putHash(hash);
			}
			copy(dst);
		}
	}
	
	void copy_interface(TypeOutput* dst) {
		while(!error()) {
			int size = 0;
			int id = copy_entry(size, dst);
			if(id == VNL_IO_INTERFACE && size == VNL_IO_END) {
				break;
			}
			copy(dst, id, size);
		}
	}
	
	void copy_call(int size, TypeOutput* dst) {
		for(int i = 0; i < size && !error(); ++i) {
			copy(dst);
		}
	}
	
};




}}

#endif /* INCLUDE_IO_TYPEINPUT_H_ */
