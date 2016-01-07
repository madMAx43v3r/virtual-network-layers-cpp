/*
 * CRC64.h
 *
 *  Created on: 16.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_CRC64_H_
#define INCLUDE_CRC64_H_

#include <stdint.h>
#include <limits.h>

namespace vnl {

class CRC64 {
public:
	
	CRC64() {
		crc = -1;
	}
	
	void update(char b) {
		crc = crcTable[(b ^ (int)crc) & 0xFF] ^ (crc >> 8);
	}
	
	void update(uint64_t v) {
		for(int i = 0; i < sizeof(v); ++i) {
			update((char)(v >> (i*8)));
		}
	}
	
	void update(const char* buf, int length) {
		update(buf, 0, length);
	}
	
	void update(const char* buf, int off, int len) {
		int end = off + len;
		while (off < end) {
			crc = crcTable[(buf[off++] ^ (int)crc) & 0xFF] ^ (crc >> 8);
		}
	}
	
	uint64_t getValue() {
		return ~crc;
	}
	
private:
	uint64_t crc;
	
	static uint64_t poly;
	static uint64_t crcTable[256];
	
	struct init;
	static init initializer;
	
	
};

struct CRC64::init {
	init() {
		for (int b = 0; b < 256; ++b) {
			uint64_t r = b;
			for (int i = 0; i < 8; ++i) {
					if ((r & 1) == 1) {
							r = (r >> 1) ^ poly;
					} else {
							r >>= 1;
					}
			}
			crcTable[b] = r;
		}
	}
};

uint64_t CRC64::poly = 0xC96C5795D7870F42ull;
uint64_t CRC64::crcTable[256];

CRC64::init CRC64::initializer;

}

#endif /* INCLUDE_CRC64_H_ */
