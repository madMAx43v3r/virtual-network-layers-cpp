/*
 * types.h
 *
 *  Created on: Dec 13, 2015
 *      Author: mad
 */

#ifndef INCLUDE_PLATFORM_POSIX_TYPES_H_
#define INCLUDE_PLATFORM_POSIX_TYPES_H_

#include <stdint.h>
#include <arpa/inet.h>
#include <endian.h>

inline uint16_t vnl_htons(uint16_t v) {
	return htons(v);
}
inline uint32_t vnl_htonl(uint32_t v) {
	return htonl(v);
}
inline uint64_t vnl_htonll(uint64_t v) {
	return htobe64(v);
}
inline float vnl_htonf(float v) {
	uint32_t tmp = vnl_htonl(*((uint32_t*)&v));
	return *((float*)&tmp);
}
inline double vnl_htond(double v) {
	uint64_t tmp = vnl_htonll(*((uint64_t*)&v));
	return *((double*)&tmp);
}

inline uint16_t vnl_ntohs(uint16_t v) {
	return ntohs(v);
}
inline uint32_t vnl_ntohl(uint32_t v) {
	return ntohl(v);
}
inline uint64_t vnl_ntohll(uint64_t v) {
	return be64toh(v);
}
inline float vnl_ntohf(uint32_t v) {
	uint32_t tmp = vnl_ntohl(v);
	return *((float*)&tmp);
}
inline double vnl_ntohd(uint64_t v) {
	uint64_t tmp = vnl_ntohll(v);
	return *((double*)&tmp);
}

#endif /* INCLUDE_PLATFORM_POSIX_TYPES_H_ */
