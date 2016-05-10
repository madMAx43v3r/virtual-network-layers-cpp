/*
 * Uplink.h
 *
 *  Created on: 17.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_UPLINK_H_
#define INCLUDE_UPLINK_H_

#include "Node.h"
#include "Packet.h"

namespace vnl {

class Uplink : public Node {
public:
	Uplink(Uplink* uplink) : Node::Node(uplink) {}
	
	typedef phy::SignalType<0x85490083> connect_t;
	typedef phy::SignalType<0x06824d17> disconnect_t;
	
};


}

#endif /* INCLUDE_UPLINK_H_ */
