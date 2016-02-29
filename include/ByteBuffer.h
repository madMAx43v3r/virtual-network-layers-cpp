/*
 * ByteBuffer.h
 *
 *  Created on: Jan 8, 2016
 *      Author: mad
 */

#ifndef INCLUDE_BYTEBUFFER_H_
#define INCLUDE_BYTEBUFFER_H_

#include <cstdlib>
#include <stdint.h>
#include <string.h>
#include "util/types.h"
#include "io/Stream.h"

namespace vnl {

class ByteBuffer {
public:
	ByteBuffer(vnl::io::Stream* stream);
	
	char get();
	void get(void* buf, int len);
	int8_t getChar();
	double getDouble();
	float getFloat();
	int32_t getInt();
	int64_t getLong();
	int16_t getShort();
	
	void put(char b);
	void put(const void* buf, int len);
	void putChar(int8_t value);
	void putDouble(double value);
	void putFloat(float value);
	void putInt(int32_t value);
	void putLong(int64_t value);
	void putShort(int16_t value);
	
	bool error = false;
	
private:
	vnl::io::Stream* stream;
	
	template<typename T>
	T read() {
		T data;
		error |= stream->read(&data, sizeof(T)) != sizeof(T);
		return data;
	}
	
	template<typename T>
	void append(T data) {
		error |= stream->write(&data, sizeof(T));
	}
	
};


char ByteBuffer::get() {
	return read<char>();
}

void ByteBuffer::get(void* buf, int len) {
	stream->read(buf, len);
}

int8_t ByteBuffer::getChar() {
	return read<int8_t>();
}

double ByteBuffer::getDouble() {
	return vnl_ntohd(read<double>());
}

float ByteBuffer::getFloat() {
	return vnl_ntohf(read<float>());
}

int32_t ByteBuffer::getInt() {
	return vnl_ntohl(read<int32_t>());
}

int64_t ByteBuffer::getLong() {
	return vnl_ntohll(read<int64_t>());
}

int16_t ByteBuffer::getShort() {
	return vnl_ntohs(read<int16_t>());
}


void ByteBuffer::put(char b) {
	append<char>(b);
}

void ByteBuffer::put(const void* buf, int len) {
	stream->write(buf, len);
}

void ByteBuffer::putChar(int8_t value) {
	append<int8_t>(value);
}

void ByteBuffer::putDouble(double value) {
	append<double>(vnl_htond(value));
}

void ByteBuffer::putFloat(float value) {
	append<float>(vnl_htonf(value));
}

void ByteBuffer::putInt(int32_t value) {
	append<int32_t>(vnl_htonl(value));
}

void ByteBuffer::putLong(int64_t value) {
	append<int64_t>(vnl_htonll(value));
}

void ByteBuffer::putShort(int16_t value) {
	append<int16_t>(vnl_htons(value));
}


}

#endif
