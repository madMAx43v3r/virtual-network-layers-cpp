/*
 * Stream.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_STREAM_H_
#define INCLUDE_PHY_STREAM_H_

#include <vnl/Pipe.h>
#include <vnl/Engine.h>
#include <vnl/Queue.h>
#include <vnl/Router.h>
#include <vnl/Layer.h>


namespace vnl {

class Stream : public Basic {
public:
	Stream() : engine(0), next_seq(1) {}
	
	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;
	
	~Stream() {
		if(engine) {
			close();
		}
	}
	
	void connect(Engine* engine_) {
		engine = engine_;
		Pipe::connect_t msg(this);
		send(&msg, Router::instance);
	}
	
	void close() {
		flush();
		Pipe::close_t msg(this);
		send(&msg, Router::instance);
		Message* left = 0;
		while(queue.pop(left)) {
			left->ack();
		}
		engine = 0;
	}
	
	Engine* get_engine() const {
		return engine;
	}
	
	// thread safe
	virtual void receive(Message* msg) {
		assert(engine);
		if(msg->gate == engine) {
			push(msg);
		} else {
			engine->receive(msg);
		}
	}
	
	Address subscribe(Address addr) {
		Router::open_t msg(vnl::make_pair(mac, addr));
		send(&msg, Router::instance);
		return addr;
	}
	
	void unsubscribe(Address addr) {
		Router::close_t msg(vnl::make_pair(mac, addr));
		send(&msg, Router::instance);
	}
	
	void send(Message* msg, Basic* dst) {
		assert(engine);
		msg->src = this;
		engine->send(msg, dst);
	}
	
	void send_async(Message* msg, Basic* dst) {
		assert(engine);
		msg->src = this;
		engine->send_async(msg, dst);
	}
	
	void send(Packet* packet, Address dst) {
		packet->seq_num = next_seq++;
		packet->src_mac = mac;
		packet->dst_addr = dst;
		send(packet, Router::instance);
	}
	
	void send_async(Packet* packet, Address dst) {
		packet->seq_num = next_seq++;
		packet->src_mac = mac;
		packet->dst_addr = dst;
		send_async(packet, Router::instance);
	}
	
	void flush() {
		assert(engine);
		engine->flush();
	}
	
	Message* poll() {
		return poll(-1);
	}
	
	Message* poll(int64_t micros) {
		assert(engine);
		Message* msg = 0;
		if(!queue.pop(msg)) {
			if(engine->poll(this, micros)) {
				queue.pop(msg);
			}
		}
		return msg;
	}
	
	void push(Message* msg) {
		queue.push(msg);
	}
	
	bool empty() const {
		return queue.empty();
	}
	
private:
	Engine* engine;
	Queue<Message*> queue;
	uint32_t next_seq;
	
	friend class Engine;
	
};



}

#endif /* INCLUDE_PHY_STREAM_H_ */
