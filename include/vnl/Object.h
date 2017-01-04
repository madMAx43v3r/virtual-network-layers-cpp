/*
 * Object.h
 *
 *  Created on: Dec 16, 2015
 *      Author: mad
 */

#ifndef INCLUDE_VNL_OBJECT_H_
#define INCLUDE_VNL_OBJECT_H_

#include <vnl/Layer.h>
#include <vnl/Engine.h>
#include <vnl/Stream.h>
#include <vnl/Pool.h>
#include <vnl/Timer.h>
#include <vnl/String.h>
#include <vnl/Sample.h>
#include <vnl/Frame.h>
#include <vnl/Client.h>

#include <vnl/ObjectSupport.hxx>
#include <vnl/info/ClientInfo.hxx>


namespace vnl {

class Object : public ObjectBase, public Basic {
public:
	Object(const vnl::String& domain, const vnl::String& topic)
		:	ObjectBase(domain, topic),
			vnl_dorun(false), vnl_engine(0), vnl_proxy(0), vnl_channel(0),
			my_domain(domain), my_topic(topic),
			my_address(domain, topic),
			vnl_input(&vnl_buf_in), vnl_output(&vnl_buf_out),
			vnl_spawn_time(0), vnl_log_writer(this)
	{
		my_private_address = Address(my_address.domain(), vnl_stream.get_mac());
		my_private_domain << my_domain << "." << my_topic;
	}
	
	// thread safe
	virtual void receive(Message* msg) {
		vnl_stream.receive(msg);
	}
	
	// thread safe
	uint64_t get_mac() const {
		return vnl_stream.get_mac();
	}
	
	// NOT thread safe
	Address get_my_address() const {
		assert(vnl_dorun == false);
		return my_address;
	}
	
	// NOT thread safe
	Address get_my_private_address() const {
		assert(vnl_dorun == false);
		return my_private_address;
	}
	
	// NOT thread safe
	String get_my_domain() const {
		assert(vnl_dorun == false);
		return my_domain;
	}
	
	// NOT thread safe
	String get_my_private_domain() const {
		assert(vnl_dorun == false);
		return my_private_domain;
	}
	
	// NOT thread safe
	String get_my_topic() const {
		assert(vnl_dorun == false);
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
	
	/*
	 * Returns the proxy mac through which the current packet came in.
	 */
	uint64_t get_proxy() const {
		return vnl_proxy;
	}
	
	/*
	 * Returns the input channel through which the current message came in.
	 */
	Stream* get_channel() const {
		return vnl_channel;
	}
	
	void add_client(Client& client, Router* target = 0);
	void add_input(Stream& stream, Router* target = 0);
	void add_input(InputPin& pin);
	void add_output(OutputPin& pin);
	
	Address subscribe(const String& domain, const String& topic);
	Address subscribe(Address address);
	
	void unsubscribe(Hash64 domain, Hash64 topic);
	void unsubscribe(Address address);
	
	void publish(Value* data, const String& domain, const String& topic, bool no_drop = false);
	void publish(Value* data, Address topic, bool no_drop = false);
	
	void send(Packet* packet, Address dst, bool no_drop = false);
	void send_async(Packet* packet, Address dst, bool no_drop = false);
	
	void send(Packet* packet, Basic* dst, bool no_drop = false);
	void send_async(Packet* packet, Basic* dst, bool no_drop = false);
	
	void send(Message* msg, Basic* dst, bool no_drop = false);
	void send_async(Message* msg, Basic* dst, bool no_drop = false);
	
	StringWriter log(int level);
	
	Timer* set_timeout(int64_t micros, const std::function<void()>& func, int type);
	
	bool poll(int64_t micros);
	
	bool sleep(int64_t secs);
	bool usleep(int64_t micros);
	
	virtual void run();
	
	virtual bool handle(Message* msg);
	virtual bool handle(Stream::notify_t* msg);
	virtual bool handle(OutputPin::pin_data_t* msg);
	virtual bool handle(Packet* pkt);
	virtual bool handle(Sample* sample);
	virtual bool handle(Frame* frame);
	
	virtual Frame* exec_vni_call(Frame* frame);
	
	void exit();
	String get_private_domain() const;
	Map<String, String> get_config_map() const;
	String get_config(const Hash32& name) const;
	void set_config(const Hash32& name, const String& value);
	
	void handle(const Shutdown& event);
	
protected:
	volatile bool vnl_dorun;
	MessagePool<Sample> vnl_sample_buffer;
	MessagePool<Frame> vnl_frame_buffer;
	
	Address my_address;
	Address my_private_address;
	String my_private_domain;
	String my_domain;
	String my_topic;
	
private:
	void exec(Engine* engine, Message* msg, Pipe* pipe);
	void heartbeat();
	
private:
	Stream vnl_stream;
	Engine* vnl_engine;
	uint64_t vnl_proxy;
	Stream* vnl_channel;
	
	List<Timer> vnl_timers;
	
	Map<Hash64, int64_t> vnl_sources;
	Map<Hash64, int64_t> vnl_input_nodes;
	Map<Hash64, int64_t> vnl_input_channels;
	Map<Hash64, String> vnl_output_channels;
	Map<Hash64, InputPin*> vnl_input_pins;
	Map<Hash64, OutputPin*> vnl_output_pins;
	Map<Hash64, vnl::info::ClientInfo> vnl_clients;
	
	vnl::io::ByteBuffer vnl_buf_in;
	vnl::io::ByteBuffer vnl_buf_out;
	vnl::io::TypeInput vnl_input;
	vnl::io::TypeOutput vnl_output;
	
	int64_t vnl_spawn_time;
	GlobalLogWriter vnl_log_writer;
	
	friend class Engine;
	friend class GlobalLogWriter;
	
};



} // vnl

#endif /* INCLUDE_VNL_OBJECT_H_ */
