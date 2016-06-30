/*
 * Stream.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_STREAM_H_
#define INCLUDE_PHY_STREAM_H_

#include <vnl/Basic.h>
#include "vnl/Engine.h"
#include "vnl/Memory.h"
#include "vnl/Queue.h"


namespace vnl {

class Stream : public Basic {
public:
	typedef MessageType<Stream*, 0xe39e616f> signal_t;
	
	Stream() : engine(0), mac(0) {}
	
	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;
	
	void open(Engine* engine_) {
		engine = engine_;
		mac = engine->rand();
	}
	
	// thread safe
	virtual void receive(Message* msg) override {
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
	
	void send(Message* msg, Basic* dst) {
		engine->send(msg, dst);
	}
	
	void send_async(Message* msg, Basic* dst) {
		engine->send_async(msg, dst);
	}
	
	void flush() {
		engine->flush();
	}
	
	void listen(Basic* dst) {
		listener = dst;
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
		if(listener) {
			send_async(engine->buffer.create<signal_t>(), listener);
			listener = 0;
		}
	}
	
private:
	Engine* engine;
	Queue<Message*> queue;
	Basic* listener = 0;
	
	friend class Engine;
	
};



}

#endif /* INCLUDE_PHY_STREAM_H_ */
