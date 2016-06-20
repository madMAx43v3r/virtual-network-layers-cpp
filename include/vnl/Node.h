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
#include "vnl/Layer.h"


namespace vnl {

class Node;
class Receiver;


struct log_msg_t {
	uint64_t node;
	const String* msg;
	VNL_SAMPLE(log_msg_t);
};


class GlobalLogWriter : public StringOutput {
public:
	GlobalLogWriter(Node* node) : node(node) {}
	virtual void write(const String& str) override;
private:
	Node* node;
};


class Node : public Module {
public:
	Node() : Module(), log_writer(this) { init(); }
	
	Node(uint64_t mac) : Module(mac), log_writer(this) { init(); }
	
	Node(const char* name) : Module(name), log_writer(this) { init(); }
	
	Node(const String& name) : Module(name), log_writer(this) { init(); }
	
protected:
	void open(Address address) {
		Router::open_t msg(std::make_pair(this, address));
		Module::send(&msg, Router::instance);
	}
	void close(Address address) {
		Router::close_t msg(std::make_pair(this, address));
		Module::send(&msg, Router::instance);
	}
	
	void send(Message* msg, Basic* dst) {
		Module::send(msg, dst);
	}
	void send_async(Message* msg, Basic* dst) {
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
	
	void send(Packet* packet, Address dst) {
		packet->dst_addr = dst;
		Module::send(packet, Router::instance);
	}
	void send_async(Packet* packet, Address dst) {
		packet->dst_addr = dst;
		Module::send_async(packet, Router::instance);
	}
	
	virtual bool handle(Message* msg) override {
		if(msg->msg_id == Packet::MID) {
			Packet* pkt = (Packet*)msg;
			return handle(pkt);
		}
		return false;
	}
	
	virtual bool handle(Packet* pkt) { return false; }
	
private:
	GlobalLogWriter log_writer;
	
	void init() {
		log_output = &log_writer;
	}
	
	friend class Receiver;
	friend class GlobalLogWriter;
	
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
