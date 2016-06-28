/*
 * TypeInput.h
 *
 *  Created on: May 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_TYPEINPUT_H_
#define INCLUDE_IO_TYPEINPUT_H_

#include <vnl/io/ByteInput.h>


namespace vnl { namespace io {

template<typename TStream>
class TypeInput : public ByteInput<TStream> {
public:
	typedef ByteInput<TStream> Base;
	
	TypeInput(TStream& stream) : ByteInput<TStream>(stream) {}
	
	void getHash(uint32_t& hash) {
		Base::getInt(hash);
	}
	
	void getSize(int32_t& size) {
		int8_t tmp;
		Base::getChar(tmp);
		if(tmp == -128) {
			Base::getInt(size);
		} else {
			size = tmp;
		}
	}
	
	void getChar(int8_t& value, int32_t size) {
		switch(size) {
			case 1: { Base::getChar(value); break; }
			case 2: { int16_t tmp; Base::getShort(tmp); value = tmp; break; }
			case 4: { int32_t tmp; Base::getInt(tmp); value = tmp; break; }
			case 8: { int64_t tmp; Base::getLong(tmp); value = tmp; break; }
			default: Base::err = true;
		}
	}
	
	void getShort(int16_t& value, int32_t size) {
		switch(size) {
			case 1: { int8_t tmp; Base::getChar(tmp); value = tmp; break; }
			case 2: { Base::getShort(value); break; }
			case 4: { int32_t tmp; Base::getInt(tmp); value = tmp; break; }
			case 8: { int64_t tmp; Base::getLong(tmp); value = tmp; break; }
			default: Base::err = true;
		}
	}
	
	void getInt(int32_t& value, int32_t size) {
		switch(size) {
			case 1: { int8_t tmp; Base::getChar(tmp); value = tmp; break; }
			case 2: { int16_t tmp; Base::getShort(tmp); value = tmp; break; }
			case 4: { Base::getInt(value); break; }
			case 8: { int64_t tmp; Base::getLong(tmp); value = tmp; break; }
			default: Base::err = true;
		}
	}
	
	void getLong(int64_t& value, int32_t size) {
		switch(size) {
			case 1: { int8_t tmp; Base::getChar(tmp); value = tmp; break; }
			case 2: { int16_t tmp; Base::getShort(tmp); value = tmp; break; }
			case 4: { int32_t tmp; Base::getInt(tmp); value = tmp; break; }
			case 8: { Base::getLong(value); break; }
			default: Base::err = true;
		}
	}
	
	void getFloat(float& value, int32_t size) {
		switch(size) {
			case 4: { Base::getFloat(value); break; }
			case 8: { double tmp; Base::getDouble(tmp); value = tmp; break; }
			default: Base::err = true;
		}
	}
	
	void getDouble(double& value, int32_t size) {
		switch(size) {
			case 4: { float tmp; Base::getFloat(tmp); value = tmp; break; }
			case 8: { Base::getDouble(value); break; }
			default: Base::err = true;
		}
	}
	
	void getBinary(Page* buf, int32_t len) {
		Base::getBinary(buf, len);
	}
	
	void getString(vnl::String& str, int32_t len) {
		Base::getString(str, len);
	}
	
	void skip(int32_t size) {
		if(size < 0) {
			int n = -size;
			for(int i = 0; i < n && !Base::err; ++i) {
				uint32_t hash;
				getHash(hash);
				getSize(size);
				skip(size);
			}
		} else {
			char buf[1024];
			while(size > 0 && !Base::err) {
				int32_t n = std::min(size, 1024);
				Base::get(buf, n);
				size -= n;
			}
		}
	}
	
};




}}

#endif /* INCLUDE_IO_TYPEINPUT_H_ */
