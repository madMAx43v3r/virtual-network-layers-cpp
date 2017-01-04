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
		assert(engine == 0);
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
	
	void connect(Engine* engine_, Router* target_ = 0) {
		engine = engine_;
		target = target_ ? target_ : Router::instance;
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
			for(auto& entry : table) {
				Router::close_t msg(vnl::make_pair(vnl_mac, entry.first));
				send(&msg, target);
			}
			Router::finish_t msg(this);
			send(&msg, target);
		}
		if(!engine) {
			assert(queue.empty());
		}
		Message* left = 0;
		while(queue.pop(left)) {
			left->ack();
		}
		engine = 0;
		target = 0;
		listener = 0;
	}
	
	Engine* get_engine() const {
		return engine;
	}
	
	void set_timeout(int64_t to_usec) {
		send_timeout = to_usec;
	}
	
	Address subscribe(Address addr) {
		assert(target);
		int64_t& count = table[addr];
		if(count == 0) {
			Router::open_t msg(vnl::make_pair(vnl_mac, addr));
			send(&msg, target);
		}
		count++;
		return addr;
	}
	
	void unsubscribe(Address addr) {
		assert(target);
		int64_t& count = table[addr];
		if(count > 0) {
			count--;
			if(count == 0) {
				Router::close_t msg(vnl::make_pair(vnl_mac, addr));
				send(&msg, target);
			}
		}
	}
	
	void send(Message* msg, Basic* dst, bool no_drop = false) {
		assert(engine);
		msg->src_mac = vnl_mac;
		msg->timeout = send_timeout;
		msg->is_no_drop = no_drop;
		engine->send(msg, dst);
	}
	
	void send_async(Message* msg, Basic* dst, bool no_drop = false) {
		assert(engine);
		msg->src_mac = vnl_mac;
		msg->timeout = send_timeout;
		msg->is_no_drop = no_drop;
		engine->send_async(msg, dst);
	}
	
	void send(Packet* pkt, Basic* dst, bool no_drop = false) {
		pkt->seq_num = next_seq++;
		send((Message*)pkt, dst, no_drop);
	}
	
	void send_async(Packet* pkt, Basic* dst, bool no_drop = false) {
		pkt->seq_num = next_seq++;
		send_async((Message*)pkt, dst, no_drop);
	}
	
	void send(Packet* pkt, Address dst, bool no_drop = false) {
		assert(target);
		pkt->dst_addr = dst;
		send(pkt, target, no_drop);
	}
	
	void send_async(Packet* pkt, Address dst, bool no_drop = false) {
		assert(target);
		pkt->dst_addr = dst;
		send_async(pkt, target, no_drop);
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
			send_async(msg, listener, true);
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
	Router* target;
	Basic* listener;
	Queue<Message*> queue;
	Map<Address, int64_t> table;
	MessagePool<notify_t> notify_buffer;
	int64_t send_timeout = 1000000;
	int64_t next_seq;
	
	friend class Engine;
	
};


} // vnl

#endif /* INCLUDE_VNL_STREAM_H_ */
