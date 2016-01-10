/*
 * Link.h
 *
 *  Created on: Dec 23, 2015
 *      Author: mad
 */

#ifndef INCLUDE_LINK_H_
#define INCLUDE_LINK_H_

#include <thread>
#include <mutex>
#include <iostream>

#include "Object.h"
#include "System.h"
#include "util/simple_queue.h"

namespace vnl { namespace phy {

class Link : public vnl::phy::Object, public vnl::Runnable {
public:
	Link(Engine* engine);
	~Link();
	
	static thread_local Link* local;
	
	void start(int core = -1);
	void stop();
	
	void run() override;
	
	int debug = 0;
	
protected:
	bool dorun = true;
	std::mutex mutex;
	
	typedef Generic<int, 0x2fe94e9f> shutdown_t;
	
	void lock() {
		mutex.lock();
	}
	
	void unlock() {
		mutex.unlock();
	}
	
	Message* poll();
	
	virtual void notify() = 0;
	virtual void wait(int millis) = 0;
	
	virtual bool startup() { return true; }
	virtual void shutdown() {}
	
private:
	void receive(Message* msg, Object* src) override;
	
	void entry() {
		run();
	}
	
private:
	vnl::util::simple_queue<Message*> queue;
	vnl::util::simple_queue<Message*> acks;
	
	std::thread* thread;
	int core_id;
	
};


}}

#endif /* INCLUDE_LINK_H_ */
