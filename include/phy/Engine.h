/*
 * Engine.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ENGINE_H_
#define INCLUDE_PHY_ENGINE_H_

#include <unordered_set>
#include <mutex>
#include <condition_variable>

#include "phy/Message.h"
#include "phy/Node.h"
#include "phy/AtomicQueue.h"
#include "phy/Memory.h"
#include "phy/RingBuffer.h"
#include "phy/Random.h"
#include "System.h"
#include "Util.h"


namespace vnl { namespace phy {

class Stream;
class Object;
class Reference;


class Engine : public Node {
public:
	Engine();
	Engine(const std::string& name);
	virtual ~Engine();
	
	static thread_local Engine* local;
	
	// thread safe
	virtual void receive(Message* msg) override {
		if(!msg->dst || msg->dst == this) {
			msg->ack();
		} else {
			msg->gate = this;
			queue.push(msg);
			if(waiting-- == 1) {
				mutex.lock();
				cond.notify_all();
				mutex.unlock();
			}
		}
	}
	
	virtual void exec(Object* object);
	
protected:
	bool dorun = true;
	
	uint64_t rand() {
		return generator.rand();
	}
	
	template<typename T>
	void send(T&& msg, Node* dst) {
		send_impl(&msg, dst, false);
	}
	
	template<typename T>
	void send_async(T&& msg, Node* dst) {
		send_async(&msg, dst);
	}
	
	template<typename T>
	void send_async(T* msg, Node* dst) {
		RingBuffer::entry_t* entry;
		T* cpy = buffer.create<T>(entry);
		*cpy = *msg;
		cpy->user = entry;
		cpy->callback = &async_cb;
		send_impl(cpy, dst, true);
	}
	
	template<typename T, typename R>
	T request(R&& req, Node* dst) {
		send(req, dst);
		return req.res;
	}
	
	void send(Message* msg, Node* dst) {
		send_impl(msg, dst, false);
	}
	
	void send_async(Message* msg, Node* dst) {
		send_impl(msg, dst, true);
	}
	
	Message* collect(int64_t timeout) {
		Message* msg = 0;
		if(queue.pop(msg)) {
			return msg;
		}
		if(timeout != 0) {
			{
				std::unique_lock<std::mutex> ulock(mutex);
				waiting = 1;
				if(!queue.pop(msg)) {
					if(timeout > 0) {
						cond.wait_for(ulock, std::chrono::microseconds(timeout));
					} else {
						cond.wait(ulock);
					}
				}
				waiting = 0;
			}
			queue.pop(msg);
		}
		return msg;
	}
	
	virtual void send_impl(Message* msg, Node* dst, bool async) = 0;
	
	virtual bool poll(Stream* stream, int64_t micros) = 0;
	
	virtual void flush() = 0;
	
	virtual void fork(Object* object) = 0;
	
private:
	void async_ack(Message* msg) {
		buffer.destroy<Message>((RingBuffer::entry_t*)msg->user);
	}
	
protected:
	Region memory;
	
private:
	std::mutex mutex;
	std::condition_variable cond;
	
	std::atomic<int> waiting;
	AtomicQueue<Message*> queue;
	
	Random64 generator;
	
	RingBuffer buffer;
	std::function<void(Message*)> async_cb;
	
	friend class Stream;
	friend class Object;
	friend class Reference;
	
};




}}

#endif /* INCLUDE_PHY_ENGINE_H_ */
