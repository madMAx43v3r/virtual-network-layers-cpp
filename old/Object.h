/*
 * Node.h
 *
 *  Created on: Jan 8, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_OBJECT_H_
#define INCLUDE_OBJECT_H_

#include "Node.h"
#include "Struct.h"
#include "Uplink.h"

namespace vnl {

class Object : public Node, Struct {
public:
	Object(Uplink* uplink);
	
protected:
	virtual void registered(const Address& src, const Address& addr) {}
	virtual void unregistered(const Address& src, const Address& addr) {}
	
private:
	virtual bool handle(phy::Message* msg) override;
	
private:
	
};


}

#endif /* INCLUDE_OBJECT_H_ */
