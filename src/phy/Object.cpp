/*
 * Object.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include "phy/Object.h"
#include <assert.h>

namespace vnl { namespace phy {

Object::Object() : engine(Engine::local) {
	if(!engine) {
		printf("ERROR: creating vnl::phy::Object without thread local engine.\n");
		assert(Engine::local != 0);
	}
	mac = rand();
}

Object::Object(Engine* engine) : engine(engine) {
	mac = rand();
}


}}
