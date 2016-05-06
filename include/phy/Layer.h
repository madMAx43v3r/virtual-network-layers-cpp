/*
 * Layer.h
 *
 *  Created on: Apr 24, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_LAYER_H_
#define INCLUDE_PHY_LAYER_H_

#include <assert.h>

#include "phy/Node.h"
#include "phy/Registry.h"
#include "phy/Router.h"


namespace vnl { namespace phy {

class Layer {
public:
	Layer() {
		assert(Registry::instance == 0);
		assert(Router::instance == 0);
		Registry::instance = new Registry();
		Router::instance = new Router();
	}
	
	~Layer() {
		SyncNode node;
		{
			Registry::shutdown_t msg;
			msg.src = &node;
			node.send(&msg, Registry::instance);
		}
		delete Router::instance;
		delete Registry::instance;
	}
	
	
};


}}

#endif /* INCLUDE_PHY_LAYER_H_ */
