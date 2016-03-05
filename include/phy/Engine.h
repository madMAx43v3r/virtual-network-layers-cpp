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
#include <unordered_set>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "util/simple_queue.h"
#include "phy/Message.h"
#include "phy/Stream.h"
#include "phy/Object.h"
#include "phy/Fiber.h"
#include "Runnable.h"
#include "System.h"

namespace vnl { namespace phy {

class Engine : public vnl::Runnable {
public:
	Engine();
	virtual ~Engine();
	
	static thread_local Engine* local;
	
	void start(int core = -1);
	void stop();
	
	void finished(Fiber* fiber) {
		avail.push_back(fiber);
	}
	
	Fiber* current = 0;
	
protected:
	bool dorun = false;
	vnl::util::simple_queue<Message*> queue;
	vnl::util::simple_queue<Message*> acks;
	std::unordered_set<Fiber*> fibers;
	std::vector<Fiber*> avail;
	int core_id;
	
	uint64_t rand() {
		return Util::hash64(generator());
	}
	
	void sent(Message* msg) {
		if(msg->impl == 0) {
			msg->impl = current;
			current->sent(msg);
		}
	}
	
	bool poll(Stream* stream, int millis) {
		stream->impl.push(current);
		bool res = current->poll(millis);
		return res;
	}
	
	void flush() {
		current->flush();
	}
	
	taskid_t launch(const std::function<void()>& func);
	
	void cancel(taskid_t task);
	
	void lock() {
		mutex.lock();
	}
	
	void unlock() {
		mutex.unlock();
	}
	
	virtual void notify() {
		cond.notify_all();
	}
	
	virtual void wait(int millis) {
		cond.wait_for(ulock, std::chrono::milliseconds(millis));
	}
	
	virtual void mainloop() = 0;
	
	virtual Fiber* create() = 0;
	
	friend class vnl::phy::Stream;
	
private:
	void entry() {
		mainloop();
	}
	
	void wait() {
		cond.wait(ulock);
	}
	
	void receive(Message* msg) {
		lock();
		if(msg->isack) {
			acks.push(msg);
		} else {
			queue.push(msg);
		}
		notify();
		unlock();
	}
	
private:
	std::mutex mutex;
	std::condition_variable cond;
	std::unique_lock<std::mutex> ulock;
	std::default_random_engine generator;
	
	std::thread* thread;
	uint32_t nextid = 1;
	
	friend class vnl::phy::Object;
	
};


}}

#endif /* INCLUDE_PHY_ENGINE_H_ */
