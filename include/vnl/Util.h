/*
 * Util.h
 *
 *  Created on: 16.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_

#include <cstdlib>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <endian.h>
#include <time.h>

#include <cxxabi.h>

#include <vnl/CRC64.h>
#include <vnl/String.h>


namespace vnl {

template<typename K, typename V>
struct pair {
	K first;
	V second;
	pair() {}
	pair(const K& key, const V& value) : first(key), second(value) {}
};

template<typename K, typename V>
inline pair<K,V> make_pair(const K& key, const V& value) {
	return pair<K,V>(key, value);
}

static int64_t currentTime() {
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

static int64_t currentTimeMillis() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

static int64_t currentTimeMicros() {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

static int64_t nanoTime() {
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

static String currentDate(const char* format) {
	::time_t timer;
	time(&timer);
	char buf[256];
	strftime(buf, sizeof(buf), format, ::localtime(&timer));
	return String() << buf;
}

static String currentDate() {
	return currentDate("%Y-%m-%d_%H:%M:%S");
}

static uint64_t hash64(const char* str) {
	CRC64 func;
	func.update(str, strlen(str));
	return func.getValue();
}

static uint64_t hash64(const std::string& str) {
	CRC64 func;
	func.update(str.c_str(), str.size());
	return func.getValue();
}

static uint64_t hash64(const String& str) {
	CRC64 func;
	for(String::const_iterator it = str.begin(); it != str.end(); ++it) {
		func.update(*it);
	}
	return func.getValue();
}

static uint64_t hash64(const char* data, size_t size) {
	CRC64 func;
	func.update(data, size);
	return func.getValue();
}

static uint64_t hash64(uint64_t a) {
	CRC64 func;
	func.update(a);
	return func.getValue();
}

template<typename T, typename R>
static uint64_t hash64(T a, R b) {
	CRC64 func;
	func.update(hash64(a)*31);
	func.update(hash64(b)*37);
	return func.getValue();
}

template<typename T, typename R, typename S>
static uint64_t hash64(T a, R b, S c) {
	CRC64 func;
	func.update(hash64(a)*31);
	func.update(hash64(b)*37);
	func.update(hash64(c)*41);
	return func.getValue();
}


inline void memcpy(void* dst, const void* src, int size) {
	switch(size) {
	case 1: *((uint8_t*)dst) = *((uint8_t*)src); break;
	case 2: *((uint16_t*)dst) = *((uint16_t*)src); break;
	case 4: *((uint32_t*)dst) = *((uint32_t*)src); break;
	case 8: *((uint64_t*)dst) = *((uint64_t*)src); break;
	default: ::memcpy(dst, src, size);
	}
}


inline bool parse_endpoint(const vnl::String* str, vnl::String& endpoint, int& port) {
	// TODO
	return false;
}


static int stick_to_core(int core_id) {
   int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
   if (core_id < 0 || core_id >= num_cores) {
	   printf("invalid core_id!\n");
	   return EINVAL;
   }
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);
   pthread_t current_thread = pthread_self();    
   return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

static std::string demangle(const char* abiName) {
	int status;    
	char* ret = abi::__cxa_demangle(abiName, 0, 0, &status);  
	std::string str(ret);
	free((void*)ret);
	return str;
}

template<typename T>
static std::string class_name(T* ptr) {
	return demangle(typeid(*ptr).name());
}



} // vnl


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


#endif /* INCLUDE_UTIL_H_ */
