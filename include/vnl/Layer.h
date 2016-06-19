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


namespace vnl {

extern Layer* layer;

class Layer : public Actor {
public:
	uint64_t domain;
	Address global_logs;
	
	Layer() {
		assert(layer == 0);
		assert(Random64::instance == 0);
		assert(Registry::instance == 0);
		assert(Router::instance == 0);
		
		layer = this;
		domain = vnl::hash64("vnl");
		global_logs = Address(domain, vnl::hash64("global_logs"));
		
		Random64::instance = new Random64();
		Registry::instance = new Registry();
		Router::instance = new Router();
	}
	
	void shutdown() {
		Registry::shutdown_t msg;
		send(&msg, Registry::instance);
	}
	
	~Layer() {
		shutdown();
		
		delete Registry::instance;
		delete Random64::instance;
		delete Router::instance;
		
		Page::cleanup();
		Block::cleanup();
	}
	
};


}

#endif /* INCLUDE_LAYER_H_ */
