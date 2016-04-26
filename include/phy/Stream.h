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

class Stream : public vnl::phy::Node {
public:
	Stream(uint64_t mac) : Stream(Engine::local, mac) {}
	
	Stream(Engine* engine, uint64_t mac)
		:	engine(engine), mem(engine), mac(mac), queue(mem), impl(mem) {}
	
	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;
	
	uint64_t MAC() const { return mac; }
	
	// thread safe
	virtual void receive(Message* msg) override {
		msg->dst = this;
		engine->receive(msg);
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
	uint64_t mac;
	Queue<Message*> queue;
	Queue<Fiber*> impl;
	
};



}}

#endif /* INCLUDE_PHY_STREAM_H_ */
