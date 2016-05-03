/*
 * ThreadEngine.h
 *
 *  Created on: May 2, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_THREADENGINE_H_
#define INCLUDE_PHY_THREADENGINE_H_

#include "Engine.h"
#include "Fiber.h"


namespace vnl { namespace phy {

class ThreadEngine : public Engine {
public:
	ThreadEngine() : fiber(this) {}
	
	virtual void run(Object* object) override {
		assert(Engine::local == this);
		fiber.exec(object);
	}
	
protected:
	
	class ThreadFiber : public Fiber {
	public:
		ThreadFiber(ThreadEngine* engine) : engine(engine) {}
		
		void exec(Object* object) {
			do_exec(engine, object);
		}
		
		virtual void sent(Message* msg, bool async) override {
			pending++;
			if(!async && pending > 0) {
				wait_msg = msg;
				while(wait_msg) {
					wait();
				}
			}
		}
		
		void acked(Message* msg) {
			if(msg->callback) {
				msg->callback(msg);
			}
			pending--;
			if(msg == wait_msg) {
				wait_msg = 0;
			}
		}
		
		virtual bool poll(int64_t millis) override {
			Message* msg = engine->collect(millis);
			if(msg) {
				if(msg->isack) {
					acked(msg);
				} else {
					Node* node = msg->dst;
					node->receive(msg);
				}
				return true;
			}
			return false;
		}
		
		virtual void flush() override {
			while(pending) {
				wait();
			}
		}
		
	private:
		void wait() {
			poll(9223372036854775808LL);
		}
		
	private:
		ThreadEngine* engine;
		int pending = 0;
		bool result = false;
		bool waiting = false;
		Message* wait_msg = 0;
		
	};
	
	virtual void fork(Object* object) override {
		std::thread thread(std::bind(&ThreadEngine::entry, object));
		thread.detach();
	}
	
private:
	
	static void entry(Object* object) {
		ThreadEngine engine;
		engine.run(object);
	}
	
private:
	ThreadFiber fiber;
	
};



}}

#endif /* INCLUDE_PHY_THREADENGINE_H_ */
