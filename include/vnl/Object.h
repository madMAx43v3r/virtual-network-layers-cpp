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

class Object : public ObjectBase {
public:
	Object(const vnl::String& domain, const vnl::String& topic)
		:	ObjectBase(domain, topic),
			dorun(false), engine(0),
			my_domain(domain), my_topic(topic),
			my_address(domain, topic),
			input(&buf_in), output(&buf_out),
			log_writer(this)
	{
	}
	
	// NOT thread safe
	Address get_my_address() const {
		assert(dorun == false);
		return my_address;
	}
	
	// NOT thread safe
	String get_my_domain() const {
		assert(dorun == false);
		return my_domain;
	}
	
	// NOT thread safe
	String get_my_topic() const {
		assert(dorun == false);
		return my_topic;
	}
	
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
	
	Stream& get_stream() {
		return stream;
	}
	
	Basic* get_basic() {
		return &stream;
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
	
protected:
	volatile bool dorun;
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
