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
#include "Queue.h"


namespace vnl { namespace phy {

class Engine;

class Stream final : public Node {
public:
	typedef Generic<Stream*, 0xe39e616f> signal_t;
	
	Stream(Engine* engine, Region* mem)
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
	
	template<typename T>
	void send(T&& msg, Node* dst) {
		send(msg, dst);
	}
	
	template<typename T>
	void send_async(T&& msg, Node* dst) {
		send_async(msg, dst);
	}
	
	template<typename T, typename R>
	T request(R&& req, Node* dst) {
		req->src = this;
		return engine->request<T>(req, dst);
	}
	
	void send(Message* msg, Node* dst) {
		msg->src = this;
		engine->send(msg, dst);
	}
	
	void send_async(Message* msg, Node* dst) {
		msg->src = this;
		engine->send_async(msg, dst);
	}
	
	void flush() {
		engine->flush();
	}
	
	void listen(Node* dst) {
		listener = dst;
	}
	
	Message* poll() {
		return poll(-1);
	}
	
	Message* poll(int64_t micros) {
		Message* msg = 0;
		if(!queue.pop(msg) && micros != 0) {
			if(engine->poll(this, micros)) {
				queue.pop(msg);
			}
		}
		return msg;
	}
	
	void push(Message* msg) {
		queue.push(msg);
		if(listener) {
			send_async(signal_t(this), listener);
			listener = 0;
		}
	}
	
	Message* pop() {
		Message* msg = 0;
		queue.pop(msg);
		return msg;
	}
	
private:
	Engine* engine;
	Queue<Message*> queue;
	Node* listener = 0;
	
	friend class Engine;
	
};



}}

#endif /* INCLUDE_PHY_STREAM_H_ */
