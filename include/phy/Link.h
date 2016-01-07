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

#include "util/simple_queue.h"
#include "Object.h"
#include "System.h"

namespace vnl { namespace phy {

class Link : public vnl::phy::Object, public vnl::Runnable {
public:
	Link(Engine* engine) : Object(engine), thread(0), core_id(-1) {}
	
	virtual ~Link();
	
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
	
	virtual void notify() = 0;
	virtual void wait(int millis) = 0;
	
	virtual bool startup() { return true; }
	virtual void shutdown() {}
	
	void receive(Message* msg, Object* src) override;
	
	Message* poll();
	
private:
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
