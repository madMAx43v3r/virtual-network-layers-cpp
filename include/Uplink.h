/*
 * Uplink.h
 *
 *  Created on: 17.11.2015
 *      Author: mwittal
 */

#ifndef INCLUDE_UPLINK_H_
#define INCLUDE_UPLINK_H_

#include "Node.h"
#include "Frame.h"

namespace vnl {

class Uplink : public Node {
public:
	Uplink(Uplink* uplink = 0) : Node(uplink) {}
	
	typedef phy::Signal<0x85490083> connect_t;
	typedef phy::Signal<0x85490083> disconnect_t;
	typedef Packet<0xef34166d> send_t;
	
protected:
	virtual void handle(phy::Message* msg) override;
	
	virtual void onconnect(phy::Message* msg) {}
	virtual void ondisconnect(phy::Message* msg) {}
	
	phy::Object* get_node(uint64_t mac) {
		auto iter = nodes.find(mac);
		if(iter != nodes.end()) {
			return iter->second;
		}
		return 0;
	}
	
	std::unordered_map<uint64_t, phy::Object*> nodes;
	
};


}

#endif /* INCLUDE_UPLINK_H_ */
