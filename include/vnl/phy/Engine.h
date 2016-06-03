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

#include "vnl/phy/Message.h"
#include "vnl/phy/Node.h"
#include "vnl/phy/Memory.h"
#include "vnl/phy/RingBuffer.h"
#include "vnl/phy/Random.h"
#include "vnl/Queue.h"
#include "vnl/Util.h"


namespace vnl { namespace phy {

class Stream;
class Object;
template<typename T> class Reference;


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
			mutex.lock();
			queue.push(msg);
			cond.notify_all();
			mutex.unlock();
		}
	}
	
	virtual void exec(Object* object);
	
protected:
	bool dorun = true;
	
	uint64_t rand() {
		return generator.rand();
	}
	
	void send(Node* src, Message* msg, Node* dst) {
		send_impl(src, msg, dst, false);
	}
	
	void send_async(Node* src, Message* msg, Node* dst) {
		send_impl(src, msg, dst, true);
	}
	
	Message* collect(int64_t timeout) {
		std::unique_lock<std::mutex> ulock(mutex);
		Message* msg = 0;
		if(queue.pop(msg)) {
			return msg;
		}
		if(timeout != 0) {
			if(timeout > 0) {
				cond.wait_for(ulock, std::chrono::microseconds(timeout));
			} else {
				cond.wait(ulock);
			}
			queue.pop(msg);
		}
		return msg;
	}
	
	virtual void send_impl(Node* src, Message* msg, Node* dst, bool async) = 0;
	
	virtual bool poll(Stream* stream, int64_t micros) = 0;
	
	virtual void flush() = 0;
	
	virtual void fork(Object* object) = 0;
	
protected:
	Region memory;
	MessageBuffer buffer;
	
private:
	std::mutex mutex;
	std::condition_variable cond;
	
	Queue<Message*> queue;
	
	Random64 generator;
	
	friend class Stream;
	friend class Object;
	template<typename T> friend class Reference;
	
};




}}

#endif /* INCLUDE_PHY_ENGINE_H_ */
