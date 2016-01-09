/*
 * Downlink.h
 *
 *  Created on: 17.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_DOWNLINK_H_
#define INCLUDE_DOWNLINK_H_

#include "phy/Object.h"
#include "Frame.h"

namespace vnl {

class Downlink : public vnl::phy::Port {
public:
	Downlink(phy::Object* obj) : Port(obj) {}
	
	typedef phy::Generic<Frame, 0xb494e3b0> receive_t;
	
};


}

#endif /* INCLUDE_DOWNLINK_H_ */
