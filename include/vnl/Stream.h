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
	Stream() : engine(0), target(0), listener(0), next_seq(1) {}
	
	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;
	
	typedef MessageType<Stream*, 0x70513a1c> notify_t;
	
	~Stream() {
		if(engine) {
			close();
		}
	}
	
	void connect(Engine* engine_, Basic* target_ = Router::instance) {
		engine = engine_;
		target = target_;
		if(target) {
			Pipe::connect_t msg(this);
			send(&msg, target);
			assert(msg.res);
		}
	}
	
	void listen(Basic* dst) {
		assert(dst != this);
		listener = dst;
	}
	
	void close() {
		if(target) {
			Pipe::close_t msg(this);
			send(&msg, target);
		}
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
			if(msg->isack) {
				msg->destroy();
			} else {
				push(msg);
			}
		} else {
			msg->dst = this;
			engine->receive(msg);
		}
	}
	
	Address subscribe(Address addr) {
		Router::open_t msg(vnl::make_pair(mac, addr));
		send(&msg, target);
		return addr;
	}
	
	void unsubscribe(Address addr) {
		Router::close_t msg(vnl::make_pair(mac, addr));
		send(&msg, target);
	}
	
	void send(Message* msg, Basic* dst) {
		assert(engine);
		engine->send(msg, dst);
	}
	
	void send_async(Message* msg, Basic* dst) {
		assert(engine);
		engine->send_async(msg, dst);
	}
	
	void send(Packet* pkt, Basic* dst) {
		pkt->seq_num = next_seq++;
		pkt->src_mac = mac;
		send((Message*)pkt, dst);
	}
	
	void send_async(Packet* pkt, Basic* dst) {
		pkt->seq_num = next_seq++;
		pkt->src_mac = mac;
		send_async((Message*)pkt, dst);
	}
	
	void send(Packet* pkt, Address dst) {
		pkt->dst_addr = dst;
		send(pkt, target);
	}
	
	void send_async(Packet* pkt, Address dst) {
		pkt->dst_addr = dst;
		send_async(pkt, target);
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
		if(listener) {
			notify_t* msg = engine->buffer.create<notify_t>();
			msg->data = this;
			send_async(msg, listener);
			listener = 0;
		}
	}
	
	bool pop(Message*& msg) {
		return queue.pop(msg);
	}
	
	bool empty() const {
		return queue.empty();
	}
	
private:
	Engine* engine;
	Basic* target;
	Basic* listener;
	Queue<Message*> queue;
	uint32_t next_seq;
	
	friend class Engine;
	
};



}

#endif /* INCLUDE_PHY_STREAM_H_ */
