/*
 * Downlink.h
 *
 *  Created on: 17.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_DOWNLINK_H_
#define INCLUDE_DOWNLINK_H_

#include "Frame.h"

namespace vnl {

class Downlink {
public:
	
	virtual ~Downlink() {}
	
	virtual void receive(Frame& frame, uint64_t dstmac) = 0;
	
};

}

#endif /* INCLUDE_DOWNLINK_H_ */
