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
#include <vnl/Pipe.h>
#include <vnl/Timer.h>
#include <vnl/String.h>
#include <vnl/Frame.h>
#include <vnl/Algorithm.h>
#include <vnl/ObjectSupport.hxx>


namespace vnl {

class Object : public Basic, public ObjectBase {
public:
	Object(const vnl::String& domain, const vnl::String& topic)
		:	ObjectBase(domain, topic),
			dorun(true), engine(0),
			my_domain(domain), my_topic(topic),
			my_address(domain, topic),
			input(&buf_in), output(&buf_out),
			log_writer(this)
	{
	}
	
 	// thread safe
	virtual void receive(Message* msg) {
		stream.receive(msg);
	}
	
	// NOT thread safe
	Address get_my_address() const {
		return my_address;
	}
	
	// NOT thread safe
	virtual void serialize(vnl::io::TypeOutput& out) const;
	
protected:
	virtual ~Object() {}
	
	virtual void main(Engine* engine, Message* init) {
		init->ack();
		main(engine);
	}
	
	virtual void main(Engine* engine) {
		main();
	}
	
	virtual void main() {
		run();
	}
	
	void attach(Pipe* pipe);
	void close(Pipe* pipe);
	
	Object* fork(Object* object);
	
	Address subscribe(const String& domain, const String& topic);
	Address subscribe(Address address);
	
	void unsubscribe(Hash64 domain, Hash64 topic);
	void unsubscribe(Address address);
	
	void publish(Value* data, const String& domain, const String& topic);
	void publish(Value* data, Address topic);
	
	void send(Packet* packet, Address dst);
	void send_async(Packet* packet, Address dst);
	
	void send(Packet* packet, Basic* dst);
	void send_async(Packet* packet, Basic* dst);
	
	void send(Message* msg, Basic* dst);
	void send_async(Message* msg, Basic* dst);
	
	void flush();
	
	StringWriter log(int level);
	
	Timer* set_timeout(int64_t micros, const std::function<void()>& func, int type);
	
	bool poll(int64_t micros);
	
	bool sleep(int64_t secs);
	bool usleep(int64_t micros);
	
	void exit();
	
	virtual void run();
	
	virtual bool handle(Message* msg);
	virtual bool handle(Packet* pkt);
	
	virtual void handle(const vnl::Shutdown& event);
	
	vnl::info::Class get_class() const;
	
	Binary vni_serialize() const;
	void vni_deserialize(const vnl::Binary& blob);
	
protected:
	volatile bool dorun;
	PageAllocator memory;
	MessagePool buffer;
	
	Address my_address;
	String my_domain;
	String my_topic;
	
private:
	void exec(Engine* engine, Message* msg, Pipe* pipe);
	
private:
	Stream stream;
	Engine* engine;
	
	List<Timer> timers;
	List<Basic*> pipes;
	
	Map<uint64_t, int64_t> sources;
	
	vnl::io::ByteBuffer buf_in;
	vnl::io::ByteBuffer buf_out;
	vnl::io::TypeInput input;
	vnl::io::TypeOutput output;
	
	GlobalLogWriter log_writer;
	
	friend class Engine;
	friend class GlobalLogWriter;
	
};



} // vnl

#endif /* INCLUDE_PHY_OBJECT_H_ */
