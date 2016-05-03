/*
 * Stream.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_STREAM_H_
#define INCLUDE_PHY_STREAM_H_

#include "phy/Node.h"
#include "phy/Engine.h"
#include "phy/Memory.h"
#include "phy/Queue.h"


namespace vnl { namespace phy {

class Engine;

class Stream : public Node {
public:
	Stream(Engine* engine)
		:	engine(engine), mem(engine), queue(mem) {}
	
	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;
	
	// thread safe
	virtual void receive(Message* msg) override {
		if(msg->safe) {
			push(msg);
		} else {
			msg->dst = this;
			engine->receive(msg);
		}
	}
	
	uint64_t rand() {
		return engine->rand();
	}
	
	template<typename T>
	void send(T&& msg, Node* dst) {
		engine->send(msg, dst);
	}
	
	template<typename T>
	void send_async(T&& msg, Node* dst) {
		engine->send_async(msg, dst);
	}
	
	template<typename T, typename R>
	T request(R&& req, Node* dst) {
		return engine->request<T>(req, dst);
	}
	
	void send(Message* msg, Node* dst) {
		engine->send(msg, dst);
	}
	
	void send_async(Message* msg, Node* dst) {
		engine->send_async(msg, dst);
	}
	
	void flush() {
		engine->flush();
	}
	
	Message* poll() {
		return poll(9223372036854775808LL);
	}
	
	Message* poll(int64_t millis) {
		Message* msg = 0;
		if(!queue.pop(msg) && millis >= 0) {
			if(engine->poll(this, millis)) {
				queue.pop(msg);
			}
		}
		return msg;
	}
	
	void push(Message* msg) {
		queue.push(msg);
	}
	
protected:
	Engine* engine;
	Region mem;
	
private:
	Queue<Message*> queue;
	
	friend class Engine;
	
};



}}

#endif /* INCLUDE_PHY_STREAM_H_ */
