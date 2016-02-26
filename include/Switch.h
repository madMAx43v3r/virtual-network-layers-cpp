/*
 * Switch.h
 *
 *  Created on: Jan 18, 2016
 *      Author: mad
 */

#ifndef INCLUDE_SWITCH_H_
#define INCLUDE_SWITCH_H_

#include "Node.h"
#include "Uplink.h"

namespace vnl {

class Switch : public Uplink {
public:
	Switch(Uplink* uplink) : Uplink(uplink) {}
	virtual ~Switch() {}
	
protected:
	virtual void handle(phy::Message* msg) override;
	
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

#endif /* INCLUDE_SWITCH_H_ */
