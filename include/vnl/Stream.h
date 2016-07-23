/*
 * Stream.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_STREAM_H_
#define INCLUDE_PHY_STREAM_H_

#include <vnl/Basic.h>
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
	
	~Stream() {}
	
	void connect(Engine* engine_) {
		engine = engine_;
	}
	
	Engine* get_engine() const {
		return engine;
	}
	
	// thread safe
	virtual void receive(Message* msg) {
		if(!engine) {
			msg->ack();
		} else if(msg->gate == engine) {
			push(msg);
		} else {
			if(!msg->dst) {
				msg->dst = this;
			}
			engine->receive(msg);
		}
	}
	
	void subscribe(Address address) {
		Router::open_t msg(std::make_pair(this, address));
		send(&msg, Router::instance);
	}
	
	void unsubscribe(Address address) {
		Router::close_t msg(std::make_pair(this, address));
		send(&msg, Router::instance);
	}
	
	void send(Message* msg, Basic* dst) {
		if(!msg->src) {
			msg->src = this;
		}
		engine->send(msg, dst);
	}
	
	void send_async(Message* msg, Basic* dst) {
		if(!msg->src) {
			msg->src = this;
		}
		engine->send_async(msg, dst);
	}
	
	void send(Packet* packet, Address dst) {
		if(packet->src_addr.is_null()) {
			packet->src_addr = Address(local_domain, mac);
		}
		if(!packet->seq_num) {
			packet->seq_num = next_seq++;
		}
		packet->src_mac = mac;
		packet->dst_addr = dst;
		send(packet, Router::instance);
	}
	
	void send_async(Packet* packet, Address dst) {
		if(packet->src_addr.is_null()) {
			packet->src_addr = Address(local_domain, mac);
		}
		if(!packet->seq_num) {
			packet->seq_num = next_seq++;
		}
		if(!packet->src_mac) {
			packet->src_mac = mac;
		}
		packet->dst_addr = dst;
		send_async(packet, Router::instance);
	}
	
	void flush() {
		engine->flush();
	}
	
	Message* poll() {
		return poll(-1);
	}
	
	Message* poll(int64_t micros) {
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
	
private:
	Engine* engine;
	Queue<Message*> queue;
	uint32_t next_seq;
	
	friend class Engine;
	
};



}

#endif /* INCLUDE_PHY_STREAM_H_ */
