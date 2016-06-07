/*
 * Node.h
 *
 *  Created on: May 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_NODE_H_
#define INCLUDE_NODE_H_

#include "vnl/Module.h"
#include "vnl/Router.h"


namespace vnl {

class Receiver;


class Node : public vnl::Module {
public:
	Node() : Module() {}
	
	Node(uint64_t mac) : Module(mac) {}
	
	Node(const char* name) : Module(name) {}
	
	Node(const vnl::String& name) : Module(name) {}
	
protected:
	void open(Address address) {
		Router::open_t msg(address);
		Module::send(&msg, Router::instance);
	}
	void close(Address address) {
		Router::close_t msg(address);
		Module::send(&msg, Router::instance);
	}
	
	void send(Message* msg, Node* dst) {
		Module::send(msg, dst);
	}
	void send_async(Message* msg, Node* dst) {
		Module::send_async(msg, dst);
	}
	
	template<typename T>
	void send(Message* msg, Reference<T>& dst) {
		Module::send(msg, dst.get());
	}
	template<typename T>
	void send_async(Message* msg, Reference<T>& dst) {
		Module::send_async(msg, dst.get());
	}
	
	void send(Packet* packet) {
		Module::send(packet, Router::instance);
	}
	void send_async(Packet* packet) {
		Module::send_async(packet, Router::instance);
	}
	
	virtual bool handle(Message* msg) override {
		if(msg->msg_id == Packet::MID) {
			vnl::Packet* pkt = (vnl::Packet*)msg;
			return handle(pkt);
		}
		return false;
	}
	
	virtual bool handle(Packet* pkt) { return false; }
	
	friend class Receiver;
	
};


class Receiver {
public:
	Receiver(Node* node, const Address& addr) : node(node), address(addr) {
		node->open(address);
	}
	
	Receiver(const Receiver&) = delete;
	Receiver& operator=(const Receiver&) = delete;
	
	~Receiver() {
		node->close(address);
	}
	
private:
	Node* node;
	Address address;
	
};



}

#endif /* INCLUDE_NODE_H_ */
