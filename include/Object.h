/*
 * Node.h
 *
 *  Created on: Jan 8, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_OBJECT_H_
#define INCLUDE_OBJECT_H_

#include <set>

#include "util/simple_hashmap.h"
#include "phy/Object.h"
#include "Uplink.h"

namespace vnl {

class Object : public vnl::phy::Object, public Downlink {
public:
	Object(phy::Object* parent, Uplink* uplink) : phy::Object(parent), Downlink(this), uplink(uplink) {
		send(Uplink::connect_t(uplink->obj));
	}
	
	~Object() {
		for(auto addr : logical) {
			unregister(addr);
		}
		send(Uplink::disconnect_t(uplink->obj));
	}
	
protected:
	using phy::Message;
	using phy::Object::send;
	
	virtual void receive(const Frame& frame) = 0;
	
	void send(const Frame& frame) {
		send(Uplink::send_t(frame, uplink->obj));
	}
	
	void configure(const Address& addr) {
		send(Frame(Frame::REGISTER, addr));
		logical.insert(addr);
	}
	
	void unregister(const Address& addr) {
		send(Frame(Frame::UNREGISTER, addr));
		logical.erase(addr);
	}
	
private:
	void handle(Message* msg) override {
		if(msg->oid == oid) {
			receive(msg->cast<receive_t>()->data);
		} else {
			Object** obj = ext.get(msg->oid);
			if(obj) {
				(*obj)->handle(msg);
			}
		}
		msg->ack();
	}
	
private:
	Uplink* uplink;
	std::set<Address> logical;
	vnl::util::simple_hashmap<uint32_t, Object*> ext;
	
};


}

#endif /* INCLUDE_OBJECT_H_ */
