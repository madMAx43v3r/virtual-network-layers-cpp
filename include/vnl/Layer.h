/*
 * Layer.h
 *
 *  Created on: May 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_LAYER_H_
#define INCLUDE_LAYER_H_

#include <vnl/Basic.h>


namespace vnl {

extern uint64_t local_domain;
extern const char* local_domain_name;


class Process;

class Layer : public Actor {
public:
	Layer(const char* domain);
	
	~Layer();
	
	static volatile bool shutdown;
	
private:
	static volatile bool finished;
	
	friend class Process;
	
};



}

#endif /* INCLUDE_LAYER_H_ */
