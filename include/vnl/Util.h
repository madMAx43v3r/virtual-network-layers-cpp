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

#include <cxxabi.h>

#include "vnl/CRC64.h"
#include "vnl/String.h"


namespace vnl {

static int64_t currentTimeMillis() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

static int64_t currentTimeMicros() {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

static int64_t nanoTime() {
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
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
	auto* chunk = str.front();
	while(chunk) {
		func.update(chunk->str, chunk->len);
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

static uint64_t hash64(uint64_t a, uint64_t b) {
	CRC64 func;
	func.update(a*31);
	func.update(b*37);
	return func.getValue();
}

static uint64_t hash64(uint64_t a, uint64_t b, uint64_t c) {
	CRC64 func;
	func.update(a*31);
	func.update(b*37);
	func.update(c*41);
	return func.getValue();
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

template<typename T>
static std::string demangle(T* ptr) {
	return demangle(typeid(*ptr).name());
}

template<typename T>
static std::string demangle(const T& ptr) {
	return demangle(typeid(ptr).name());
}

static std::string demangle(const char* abiName) {
	int status;    
	char* ret = abi::__cxa_demangle(abiName, 0, 0, &status);  
	std::string str(ret);
	free((void*)ret);
	return str;
}



}

#endif /* INCLUDE_UTIL_H_ */
