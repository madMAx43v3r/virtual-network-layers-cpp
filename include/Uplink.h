/*
 * Uplink.h
 *
 *  Created on: 17.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_UPLINK_H_
#define INCLUDE_UPLINK_H_

#include "phy/Message.h"
#include "phy/Object.h"
#include "Frame.h"

namespace vnl {

class Uplink : public vnl::phy::Port {
public:
	Uplink(phy::Object* obj) : Port(obj) {}
	
	typedef phy::Signal<0x85490083> connect_t;
	typedef phy::Signal<0xa8b79ccd> disconnect_t;
	typedef phy::Generic<Frame, 0xef34166d> send_t;
	
};

}

#endif /* INCLUDE_UPLINK_H_ */
