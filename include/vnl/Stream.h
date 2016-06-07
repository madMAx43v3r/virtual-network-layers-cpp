/*
 * Stream.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_STREAM_H_
#define INCLUDE_PHY_STREAM_H_

#include "vnl/Base.h"
#include "vnl/Engine.h"
#include "vnl/Memory.h"
#include "vnl/Queue.h"


namespace vnl {

class Stream final : public Base {
public:
	typedef MessageType<Stream*, 0xe39e616f> signal_t;
	
	Stream(Engine* engine, Region& mem)
		:	engine(engine), queue(mem)
	{
		mac = engine->rand();
	}
	
	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;
	
	Engine* getEngine() const {
		return engine;
	}
	
	// thread safe
	virtual void receive(Message* msg) override {
		if(msg->gate == engine) {
			push(msg);
		} else {
			if(!msg->dst) {
				msg->dst = this;
			}
			engine->receive(msg);
		}
	}
	
	void send(Message* msg, Base* dst) {
		engine->send(this, msg, dst);
	}
	
	void send_async(Message* msg, Base* dst) {
		engine->send_async(this, msg, dst);
	}
	
	void flush() {
		engine->flush();
	}
	
	void listen(Base* dst) {
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
			send_async(engine->buffer.create<signal_t>(this), listener);
			listener = 0;
		}
	}
	
private:
	Engine* engine;
	Queue<Message*> queue;
	Base* listener = 0;
	
	friend class Engine;
	
};



}

#endif /* INCLUDE_PHY_STREAM_H_ */
