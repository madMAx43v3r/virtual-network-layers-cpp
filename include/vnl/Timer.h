/*
 * Timer.h
 *
 *  Created on: May 11, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_TIMER_H_
#define INCLUDE_PHY_TIMER_H_

#include <assert.h>


enum { VNL_TIMER_REPEAT, VNL_TIMER_MANUAL, VNL_TIMER_ONCE };

namespace vnl {

class Object;

class Timer {
public:
	Timer() : type(VNL_TIMER_MANUAL), active(true) {
		deadline = 0;
		interval = 0;
	}
	
	void reset() {
		active = true;
		deadline = currentTimeMicros() + interval;
	}
	
	void stop() {
		active = false;
	}
	
	int64_t interval;
	
private:
	int64_t deadline;
	std::function<void(Timer*)> func;
	int type;
	bool active;
	
	friend class Object;
	
};




}



#endif /* INCLUDE_PHY_TIMER_H_ */
