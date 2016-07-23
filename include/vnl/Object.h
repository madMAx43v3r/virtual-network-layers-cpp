/*
 * Object.h
 *
 *  Created on: Dec 16, 2015
 *      Author: mad
 */

#ifndef INCLUDE_PHY_OBJECT_H_
#define INCLUDE_PHY_OBJECT_H_

#include <vnl/Engine.h>
#include <vnl/Stream.h>
#include <vnl/Pool.h>
#include <vnl/Timer.h>
#include <vnl/String.h>
#include <vnl/Frame.h>
#include <vnl/ObjectSupport.hxx>


namespace vnl {

class Object : public Basic, public ObjectBase {
public:
	Object(const vnl::String& domain, const vnl::String& topic)
		:	dorun(true), engine(0),
			my_domain(domain), my_topic(topic),
			my_address(domain, topic),
			in(&buf_in), out(&buf_out),
			log_writer(this)
	{
	}
	
	vnl::Address my_address;
	vnl::String my_domain;
	vnl::String my_topic;
	
 	// thread safe
	virtual void receive(Message* msg) {
		stream.receive(msg);
	}
	
	virtual void serialize(vnl::io::TypeOutput& out) const;
	
protected:
	virtual ~Object() {}
	
	virtual void main(Engine* engine, Message* init) {
		init->ack();
		run();
	}
	
	Object* fork(Object* object) {
		engine->fork(object);
		return object;
	}
	
	void subscribe(Hash64 domain, Hash64 topic) {
		subscribe(Address(domain, topic));
	}
	
	void subscribe(Address address) {
		Router::open_t msg(std::make_pair(this, address));
		send(&msg, Router::instance);
		ifconfig.push_back(address);
	}
	
	void unsubscribe(Address address) {
		Router::close_t msg(std::make_pair(this, address));
		send(&msg, Router::instance);
	}
	
	void publish(Value* data, Hash64 domain, Hash64 topic) {
		publish(data, Address(domain, topic));
	}
	
	void publish(Value* data, Address topic);
	
	void send(Packet* packet, Address dst) {
		if(!packet->src) {
			packet->src = this;
		}
		if(packet->src_addr.is_null()) {
			packet->src_addr = my_address;
		}
		stream.send(packet, dst);
	}
	
	void send_async(Packet* packet, Address dst) {
		if(!packet->src) {
			packet->src = this;
		}
		if(packet->src_addr.is_null()) {
			packet->src_addr = my_address;
		}
		stream.send_async(packet, dst);
	}
	
	void send(Message* msg, Basic* dst) {
		if(!msg->src) {
			msg->src = this;
		}
		stream.send(msg, dst);
	}
	
	void send_async(Message* msg, Basic* dst) {
		if(!msg->src) {
			msg->src = this;
		}
		stream.send_async(msg, dst);
	}
	
	void flush() {
		stream.flush();
	}
	
	StringWriter log(int level);
	
	Timer* set_timeout(int64_t micros, const std::function<void(Timer*)>& func, int type);
	
	bool poll(int64_t micros);
	
	bool sleep(int64_t secs);
	bool usleep(int64_t micros);
	
	const List<Address>& get_ifconfig() const;
	
	void exit();
	
	virtual void run();
	
	virtual bool handle(Message* msg);
	virtual bool handle(Packet* pkt);
	
	void set_log_level(int32_t level);
	vnl::info::Class get_class() const;
	
	Binary vni_serialize() const;
	void vni_deserialize(const vnl::Binary& blob);
	
protected:
	volatile bool dorun;
	PageAllocator memory;
	MessagePool buffer;
	
private:
	void exec(Engine* engine, Message* msg);
	
private:
	Stream stream;
	Engine* engine;
	
	List<Timer> timers;
	List<Address> ifconfig;
	
	vnl::Map<vnl::Address, uint32_t> sources;
	vnl::io::ByteBuffer buf_in;
	vnl::io::ByteBuffer buf_out;
	vnl::io::TypeInput in;
	vnl::io::TypeOutput out;
	
	GlobalLogWriter log_writer;
	
	friend class Engine;
	friend class GlobalLogWriter;
	
};



} // vnl

#endif /* INCLUDE_PHY_OBJECT_H_ */
