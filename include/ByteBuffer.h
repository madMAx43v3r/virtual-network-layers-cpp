/**
   ByteBuffer
   ByteBuffer.h
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

#ifndef _BYTEBUFFER_H
#define _BYTEBUFFER_H

#include <cstdlib>
#include <vector>
#include <stdint.h>
#include <string.h>

class ByteBuffer {
public:
	ByteBuffer(int size = 4096);
	
	int remaining();
	int position();
	void clear();
	void flip();
	void compact();
	void resize(int newSize);
	
	char* ptr();
	void inc(int bytes);
	
	char peek();
	char get();
	void get(char* buf, int len);
	int8_t getChar();
	double getDouble();
	float getFloat();
	int32_t getInt();
	int64_t getLong();
	int16_t getShort();
	
	void put(ByteBuffer* src);
	void put(char b);
	void put(const char* b, int len);
	void putChar(int8_t value);
	void putDouble(double value);
	void putFloat(float value);
	void putInt(int32_t value);
	void putLong(int64_t value);
	void putShort(int16_t value);
	
private:
	int pos, limit;
	std::vector<char> buf;
	
	template <typename T> T read() {
		T data = read<T>(pos);
		pos += sizeof(T);
		return data;
	}
	
	template <typename T> T read(int32_t index) const {
		if(index + sizeof(T) <= limit) {
			return *((T*)&buf[index]);
		}
		return 0;
	}
	
	template <typename T> void append(T data) {
		int32_t s = sizeof(data);
		if(buf.size() < (pos + s)) {
			buf.resize(pos + s);
		}
		memcpy(&buf[pos], (char*)&data, s);
		pos += s;
	}
	
};

#endif
