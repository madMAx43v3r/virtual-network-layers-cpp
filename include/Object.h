/*
 * Node.h
 *
 *  Created on: Jan 8, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_OBJECT_H_
#define INCLUDE_OBJECT_H_

#include <unordered_set>
#include <unordered_map>

#include "Node.h"

namespace vnl {

class Object : public Node {
public:
	Object(Uplink* uplink);
	~Object();
	
protected:
	virtual void receive(const Frame& frame) = 0;
	virtual void registered(Address src, Address addr) {}
	virtual void unregistered(Address src, Address addr) {}
	
private:
	virtual void handle(phy::Message* msg) override;
	
private:
	
};


}

#endif /* INCLUDE_OBJECT_H_ */
