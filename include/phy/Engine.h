/*
 * Engine.h
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_ENGINE_H_
#define INCLUDE_PHY_ENGINE_H_

#include <random>
#include <atomic>
#include <thread>
#include <mutex>

#include "Runnable.h"
#include "Message.h"
#include "Stream.h"
#include "Object.h"
#include "System.h"
#include "util/simple_queue.h"

namespace vnl { namespace phy {

class Engine : public vnl::phy::Object, public vnl::Runnable {
public:
	Engine();
	virtual ~Engine() {}
	
	static thread_local Engine* local;
	
	void start(int core = -1);
	void stop();
	
	virtual void run() override;
	
	virtual void send(Message* msg) = 0;
	virtual void flush() = 0;
	virtual void handle(Message* msg, Stream* stream) = 0;
	virtual void open(Stream* stream) = 0;
	virtual void close(Stream* stream) = 0;
	virtual bool poll(Stream* stream, int millis) = 0;
	virtual uint64_t launch(Runnable* task) = 0;
	virtual void cancel(uint64_t tid) = 0;
	virtual int timeout() = 0;
	
	uint64_t rand();
	
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
	
	void forward(Message* msg);
	void dispatch(Message* msg);
	void dispatch(Message* msg, Stream* stream);
	
private:
	virtual void receive(Message* msg, Object* src) override;
	
	void entry() {
		run();
	}
	
private:
	std::default_random_engine generator;
	vnl::util::simple_queue<Message*> queue;
	vnl::util::simple_queue<Message*> acks;
	
	std::thread* thread;
	int core_id;
	
};


uint64_t Engine::rand() {
	return Util::hash64(generator());
}


}}

#endif /* INCLUDE_PHY_ENGINE_H_ */
