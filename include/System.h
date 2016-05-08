/*
 * System.h
 *
 *  Created on: Dec 19, 2015
 *      Author: mad
 */

#ifndef INCLUDE_SYSTEM_H_
#define INCLUDE_SYSTEM_H_

#include <chrono>

namespace vnl {

class System {
public:
	
	static int64_t currentTimeMillis() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	
	static int64_t currentTimeMicros() {
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	
	static int64_t nanoTime() {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	
};


}

#endif /* INCLUDE_SYSTEM_H_ */
