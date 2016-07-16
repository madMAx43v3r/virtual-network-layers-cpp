/*
 * Layer.h
 *
 *  Created on: May 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_LAYER_H_
#define INCLUDE_LAYER_H_

#include <vnl/ThreadEngine.h>


namespace vnl {

extern uint64_t local_domain;
extern const char* local_domain_name;


class Layer : public Actor {
public:
	Layer(const char* domain);
	
	~Layer();
	
};



}

#endif /* INCLUDE_LAYER_H_ */
