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

class Stream : public Node {
public:
	Stream() : engine(0), router(0), listener(0), next_seq(1) {}
	
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
			} else if(msg->msg_id == Pipe::connect_t::MID) {
				Pipe* pipe = ((Pipe::connect_t*)msg)->data;
				msg->ack();
				attach(pipe);
			} else if(msg->msg_id == Pipe::close_t::MID) {
				Pipe* pipe = ((Pipe::close_t*)msg)->data;
				msg->ack();
				pipes.remove(pipe);
			} else {
				push(msg);
			}
		} else {
			msg->dst = this;
			engine->receive(msg);
		}
	}
	
	void connect(Engine* engine_, Router* target_ = Router::instance) {
		engine = engine_;
		router = target_;
		if(router) {
			Router::connect_t msg(this);
			send(&msg, router);
		}
	}
	
	void listen(Basic* target) {
		assert(target != this);
		listener = target;
	}
	
	void close() {
		if(router) {
			Router::finish_t msg(this);
			send(&msg, router);
		}
		for(Basic* pipe : pipes) {
			Pipe::reset_t msg(this);
			send(&msg, pipe);
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
	
	bool attach(Pipe* pipe) {
		Pipe::attach_t request;
		request.args = this;
		send(&request, pipe);
		if(request.res) {
			pipes.push_back(pipe);
		}
		return request.res;
	}
	
	void reset(Pipe* pipe) {
		Pipe::reset_t msg(this);
		send(&msg, pipe);
		pipes.remove(pipe);
	}
	
	void close(Pipe* pipe) {
		Pipe::close_t msg;
		send(&msg, pipe);
	}
	
	Address subscribe(Address addr) {
		Router::open_t msg(vnl::make_pair(mac, addr));
		send(&msg, router);
		return addr;
	}
	
	void unsubscribe(Address addr) {
		Router::close_t msg(vnl::make_pair(mac, addr));
		send(&msg, router);
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
		send(pkt, router);
	}
	
	void send_async(Packet* pkt, Address dst) {
		pkt->dst_addr = dst;
		send_async(pkt, router);
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
	Router* router;
	Basic* listener;
	List<Basic*> pipes;
	Queue<Message*> queue;
	MessagePool<notify_t> notify_buffer;
	uint32_t next_seq;
	
	friend class Engine;
	
};



}

#endif /* INCLUDE_PHY_STREAM_H_ */
