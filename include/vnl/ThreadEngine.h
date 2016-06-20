/*
 * ThreadEngine.h
 *
 *  Created on: May 2, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_THREADENGINE_H_
#define INCLUDE_PHY_THREADENGINE_H_

#include "vnl/Engine.h"
#include "vnl/Stream.h"
#include "vnl/Module.h"


namespace vnl {

class ThreadEngine : public Engine {
public:
	virtual void fork(Module* object) override {
		spawn(object);
	}
	
	void run(Module* object) {
		exec(object);
	}
	
	static void spawn(Module* object) {
		std::thread thread(std::bind(&ThreadEngine::entry, object));
		thread.detach();
		Registry::ping(object->getMAC());
	}
	
protected:
	
	virtual void send_impl(Message* msg, Basic* dst, bool async) override {
		assert(msg->isack == false);
		assert(dst);
		
		msg->src = this;
		dst->receive(msg);
		pending++;
		if(!async && pending > 0) {
			wait_for_ack(msg);
		}
	}
	
	virtual bool poll(Stream* stream, int64_t micros) override {
		assert(stream);
		assert(stream->getEngine() == this);
		
		if(micros > 0) {
			return timed_poll(stream, micros);
		} else {
			return simple_poll(stream, micros);
		}
	}
	
	virtual void flush() override {
		while(pending > 0) {
			Message* msg = collect(-1);
			if(msg) {
				handle(msg);
			}
		}
	}
	
private:
	void wait_for_ack(Message* snd) {
		while(true) {
			Message* msg = collect(-1);
			if(msg) {
				handle(msg);
				if(msg == snd) {
					return;
				}
			}
		}
	}
	
	bool simple_poll(Stream* stream, int64_t micros) {
		while(true) {
			Message* msg = collect(micros);
			if(msg) {
				handle(msg);
				if(msg->dst == stream && !msg->isack) {
					return true;
				}
			} else {
				break;
			}
		}
		return false;
	}
	
	bool timed_poll(Stream* stream, int64_t micros) {
		int64_t now = currentTimeMicros();
		int64_t deadline = now + micros;
		while(true) {
			int64_t to = deadline - now;
			if(to < 0) {
				return false;
			}
			Message* msg = collect(to);
			if(msg) {
				handle(msg);
				if(msg->dst == stream && !msg->isack) {
					return true;
				}
			} else {
				break;
			}
			now = currentTimeMicros();
		}
		return false;
	}
	
	void handle(Message* msg) {
		if(msg->isack) {
			if(msg->callback) {
				(*msg->callback)(msg);
			}
			msg->release();
			pending--;
		} else {
			msg->dst->receive(msg);
		}
	}
	
	static void entry(Module* object) {
		ThreadEngine engine;
		engine.exec(object);
	}
	
private:
	int pending = 0;
	
	
};


void spawn(Module* object) {
	ThreadEngine::spawn(object);
}



}

#endif /* INCLUDE_PHY_THREADENGINE_H_ */
