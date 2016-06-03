/*
 * Layer.h
 *
 *  Created on: Apr 24, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_LAYER_H_
#define INCLUDE_PHY_LAYER_H_

#include <assert.h>

#include "vnl/phy/Random.h"
#include "vnl/phy/Node.h"
#include "vnl/phy/Registry.h"
#include "vnl/String.h"


namespace vnl { namespace phy {

class Layer : public SyncNode {
public:
	Layer() {
		assert(Random64::instance == 0);
		assert(Registry::instance == 0);
		
		Random64::instance = new Random64();
		Registry::instance = new Registry();
	}
	
	void shutdown() {
		Registry::shutdown_t msg;
		send(&msg, Registry::instance);
	}
	
	~Layer() {
		delete Registry::instance;
		delete Random64::instance;
		
		vnl::phy::Page::cleanup();
	}
	
};


}}

#endif /* INCLUDE_PHY_LAYER_H_ */
