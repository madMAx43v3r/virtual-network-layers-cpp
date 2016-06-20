/*
 * Layer.h
 *
 *  Created on: May 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_LAYER_H_
#define INCLUDE_LAYER_H_

#include <assert.h>

#include "vnl/Basic.h"
#include "vnl/Random.h"
#include "vnl/Registry.h"
#include "vnl/Router.h"
#include "vnl/Node.h"


namespace vnl {

extern Layer* layer;

class Layer : public Actor {
public:
	uint64_t domain;
	Address global_logs;
	
	typedef SignalType<0xbe06cb18> pause_log_t;
	typedef SignalType<0x70d34ba9> resume_log_t;
	typedef MessageType<std::string, 0xb0a02b17> set_log_filter_t;
	
	Layer();
	~Layer();
	
	void shutdown();
	
};



}

#endif /* INCLUDE_LAYER_H_ */
