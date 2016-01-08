/*
 * Uplink.h
 *
 *  Created on: 17.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_UPLINK_H_
#define INCLUDE_UPLINK_H_

#include "Downlink.h"

namespace vnl {

class Uplink {
public:
	
	virtual ~Uplink() {}
	
	virtual void connect(Downlink* link, uint64_t srcmac) = 0;
	virtual void disconnect(uint64_t srcmac) = 0;
	
	virtual void send(Frame frame, uint64_t srcmac) = 0;
	
};

}

#endif /* INCLUDE_UPLINK_H_ */
