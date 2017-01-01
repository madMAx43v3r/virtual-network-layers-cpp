/*
 * Stream.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_STREAM_H_
#define INCLUDE_VNL_STREAM_H_

#include <vnl/Engine.h>
#include <vnl/Queue.h>
#include <vnl/Router.h>


namespace vnl {

class Stream : public Node {
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
			engine->receive(msg);
		}
	}
	
	void connect(Engine* engine_, Basic* target_ = Router::instance) {
		engine = engine_;
		target = target_;
		if(target) {
			Router::connect_t msg(this);
			send(&msg, target);
		}
	}
	
	void listen(Basic* target) {
		assert(target != this);
		listener = target;
	}
	
	void close() {
		if(target) {
			Router::finish_t msg(this);
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
	
	Address subscribe(Address addr) {
		assert(target);
		Router::open_t msg(vnl::make_pair(vnl_mac, addr));
		send(&msg, target);
		return addr;
	}
	
	void unsubscribe(Address addr) {
		assert(target);
		Router::close_t msg(vnl::make_pair(vnl_mac, addr));
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
		pkt->src_mac = vnl_mac;
		send((Message*)pkt, dst);
	}
	
	void send_async(Packet* pkt, Basic* dst) {
		pkt->seq_num = next_seq++;
		pkt->src_mac = vnl_mac;
		send_async((Message*)pkt, dst);
	}
	
	void send(Packet* pkt, Address dst) {
		assert(target);
		pkt->dst_addr = dst;
		send(pkt, target);
	}
	
	void send_async(Packet* pkt, Address dst) {
		assert(target);
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
		if(listener && queue.empty()) {
			notify_t* msg = notify_buffer.create();
			msg->data = this;
			send_async(msg, listener);
		}
		queue.push(msg);
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
	MessagePool<notify_t> notify_buffer;
	uint32_t next_seq;
	
	friend class Engine;
	
};


} // vnl

#endif /* INCLUDE_VNL_STREAM_H_ */
