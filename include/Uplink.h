/*
 * Uplink.h
 *
 *  Created on: 17.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_UPLINK_H_
#define INCLUDE_UPLINK_H_

#include "Frame.h"

namespace vnl {

class Uplink : public Node {
public:
	Uplink(Uplink* uplink) : Node(uplink) {}
	
	typedef phy::Signal<0x85490083> connect_t;
	typedef phy::Signal<0x85490083> disconnect_t;
	typedef Packet<0xef34166d> send_t;
	
};


}

#endif /* INCLUDE_UPLINK_H_ */
