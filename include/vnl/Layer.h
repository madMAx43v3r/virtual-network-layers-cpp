/*
 * Layer.h
 *
 *  Created on: May 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_LAYER_H_
#define INCLUDE_LAYER_H_

#include <assert.h>

#include <vnl/Basic.h>
#include <vnl/Random.h>
#include <vnl/Registry.h>
#include <vnl/Router.h>
#include <vnl/Module.h>
#include <vnl/Hash.h>


namespace vnl {

extern Layer* layer;

struct log_msg_t {
	uint64_t node;
	const String* msg;
	VNL_SAMPLE(log_msg_t);
};

class Layer : public Actor {
public:
	uint64_t domain;
	Address global_logs;
	
	typedef SignalType<0xbe06cb18> pause_log_t;
	typedef SignalType<0x70d34ba9> resume_log_t;
	typedef MessageType<std::string, 0xb0a02b17> set_log_filter_t;
	
	Layer();
	~Layer();
	
	void shutdown();
	
};


class Thread : public Module {
public:
	Thread(const char* name) : Module(name) {}
	
protected:
	virtual void main(Engine* engine);
	
	virtual bool handle(Message* msg);
	
	virtual bool handle(Packet* pkt);
	
private:
	void output(const String& log);
	
private:
	bool paused = false;
	bool filtering = false;
	vnl::Queue<String> queue;
	std::string grep;
	
};



}

#endif /* INCLUDE_LAYER_H_ */
