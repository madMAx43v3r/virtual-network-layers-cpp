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
		if(!have_init) {
			init();
			have_init = true;
		}
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
	
	static bool have_init;
	static void init();
	
	static uint64_t poly;
	static uint64_t crcTable[256];
	
};


} // vnl

#endif /* INCLUDE_CRC64_H_ */
