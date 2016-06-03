/*
 * CRC64.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include "vnl/util/CRC64.h"

namespace vnl { namespace util {

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


}}
