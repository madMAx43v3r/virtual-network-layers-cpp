/*
 * Timer.h
 *
 *  Created on: May 11, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_TIMER_H_
#define INCLUDE_PHY_TIMER_H_

#include <assert.h>


namespace vnl {

class Module;


class Timer {
public:
	enum type_t { REPEAT, MANUAL, ONCE };
	
	void reset() {
		assert(!free);
		active = true;
		deadline = currentTimeMicros() + interval;
	}
	
	void stop() {
		active = false;
	}
	
	void destroy() {
		stop();
		free = true;
	}
	
private:
	int64_t deadline;
	int64_t interval;
	std::function<void(Timer*)> func;
	type_t type;
	bool active = true;
	bool free = false;
	Timer* next = 0;
	
	friend class Module;
	
};




}



#endif /* INCLUDE_PHY_TIMER_H_ */
