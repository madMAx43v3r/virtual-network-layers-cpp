/*
 * Node.h
 *
 *  Created on: May 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_NODE_H_
#define INCLUDE_NODE_H_

#include "phy/Object.h"
#include "Router.h"


namespace vnl {

class Connection;


class Node : public vnl::phy::Object {
public:
	Node() : Object() {}
	
	Node(uint64_t mac) : Object(mac) {}
	
	Node(const vnl::String& name) : Object(name) {}
	
	Node(Node* parent, const vnl::String& name) : Object(parent, name) {}
	
protected:
	void open(Address address) {
		Router::open_t msg(address);
		Object::send(&msg, Router::instance);
	}
	void close(Address address) {
		Router::close_t msg(address);
		Object::send(&msg, Router::instance);
	}
	
	void send(phy::Message* msg, Node* dst) {
		Object::send(msg, dst);
	}
	void send_async(phy::Message* msg, Node* dst) {
		Object::send_async(msg, dst);
	}
	
	template<typename T>
	void send(phy::Message* msg, phy::Reference<T>& dst) {
		Object::send(msg, dst.get());
	}
	template<typename T>
	void send_async(phy::Message* msg, phy::Reference<T>& dst) {
		Object::send_async(msg, dst.get());
	}
	
	void send(Packet* packet) {
		Object::send(packet, Router::instance);
	}
	void send_async(Packet* packet) {
		Object::send_async(packet, Router::instance);
	}
	
	virtual bool handle(phy::Message* msg) override;
	
	virtual bool handle(Packet* pkt) { return false; }
	
	friend class Connection;
	
};


class Connection {
public:
	Connection(Node* node, const Address& addr) : node(node), address(addr) {
		node->open(address);
	}
	
	Connection(const Connection&) = delete;
	Connection& operator=(const Connection&) = delete;
	
	~Connection() {
		node->close(address);
	}
	
private:
	Node* node;
	Address address;
	
};



}

#endif /* INCLUDE_NODE_H_ */
