/*
 * Layer.h
 *
 *  Created on: May 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_LAYER_H_
#define INCLUDE_LAYER_H_

#include "vnl/phy/Layer.h"
#include "vnl/Router.h"


namespace vnl {

class Layer : public phy::Layer {
public:
	Layer() {
		assert(Router::instance == 0);
		Router::instance = new Router();
	}
	
	~Layer() {
		delete Router::instance;
	}
	
};


}

#endif /* INCLUDE_LAYER_H_ */
