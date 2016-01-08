/**
   ByteBuffer
   ByteBuffer.cpp
   Copyright 2011 Ramsey Kant

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "ByteBuffer.h"
#include "util/types.h"

ByteBuffer::ByteBuffer(int size) {
	pos = 0;
	limit = size;
	buf.resize(size);
}

int ByteBuffer::remaining() {
	return limit - pos;
}

int ByteBuffer::position() {
	return pos;
}

void ByteBuffer::clear() {
	pos = 0;
	limit = buf.size();
}

void ByteBuffer::resize(int newSize) {
	if(newSize > buf.size()) {
		buf.resize(newSize);
	}
}

void ByteBuffer::flip() {
	limit = pos;
	pos = 0;
}

void ByteBuffer::compact() {
	int N = remaining();
	if(pos > 0 && N > 0) {
		memmove(&buf[0], &buf[pos], N);
	}
	pos = N;
	limit = buf.size();
}

char* ByteBuffer::ptr() {
	return &buf[pos];
}

void ByteBuffer::inc(int bytes) {
	pos += bytes;
}

// Read Functions

char ByteBuffer::peek() {
	return read<char>(pos);
}

char ByteBuffer::get() {
	return read<char>();
}

void ByteBuffer::get(char* buf, int len) {
	for(int32_t i = 0; i < len; i++) {
		buf[i] = read<char>();
	}
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
	return vnl_ntohl(read<uint32_t>());
}

int64_t ByteBuffer::getLong() {
	return vnl_ntohll(read<uint64_t>());
}

int16_t ByteBuffer::getShort() {
	return vnl_ntohs(read<uint16_t>());
}


// Write Functions

void ByteBuffer::put(ByteBuffer* src) {
	int32_t len = src->remaining();
	for(int32_t i = 0; i < len; i++) {
		append<char>(src->get());
	}
}

void ByteBuffer::put(char b) {
	append<char>(b);
}

void ByteBuffer::put(const char* b, int len) {
	for(int32_t i = 0; i < len; i++) {
		append<char>(b[i]);
	}
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



