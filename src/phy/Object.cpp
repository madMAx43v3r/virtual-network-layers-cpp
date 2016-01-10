/*
 * Object.cpp
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#include "phy/Object.h"
#include "phy/Link.h"
#include <assert.h>

namespace vnl { namespace phy {

Object::Object() : link(Link::local), engine(0) {
	if(link) {
		engine = link->engine;
	} else {
		printf("ERROR: creating vnl::phy::Object without thread local link.\n");
		assert(Link::local != 0);
	}
	mac = rand();
}

Object::Object(Engine* engine) : link(this), engine(engine) {
	mac = rand();
}


}}
