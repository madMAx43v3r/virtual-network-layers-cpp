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

#include <cxxabi.h>

#include "util/CRC64.h"

namespace vnl {

class Util {
public:
	
	static uint64_t hash64(const std::string& str) {
		util::CRC64 func;
		func.update(str.c_str(), str.length());
		return func.getValue();
	}
	
	static uint64_t hash64(char* data, int size) {
		util::CRC64 func;
		func.update(data, size);
		return func.getValue();
	}
	
	static uint64_t hash64(uint64_t a) {
		util::CRC64 func;
		func.update(a);
		return func.getValue();
	}
	
	static uint64_t hash64(uint64_t a, uint64_t b) {
		util::CRC64 func;
		func.update(a*31);
		func.update(b*37);
		return func.getValue();
	}
	
	static uint64_t hash64(uint64_t a, uint64_t b, uint64_t c) {
		util::CRC64 func;
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
	
};

}

#endif /* INCLUDE_UTIL_H_ */
