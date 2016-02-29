/*
 * Runnable.h
 *
 *  Created on: 02.12.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_RUNNABLE_H_
#define INCLUDE_RUNNABLE_H_

#include <functional>

namespace vnl {

class Runnable {
public:
	virtual ~Runnable() {}
	
	virtual void run() = 0;
	
};

}

#endif /* INCLUDE_RUNNABLE_H_ */
