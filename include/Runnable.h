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

class Bind : public Runnable {
public:
	Bind(const std::function<void()>& func) : func(func) {}
	
	virtual void run() {
		func();
		delete this;
	}
	
private:
	std::function<void()> func;
	
};

}

#endif /* INCLUDE_RUNNABLE_H_ */
