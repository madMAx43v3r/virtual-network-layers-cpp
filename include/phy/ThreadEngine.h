/*
 * ThreadEngine.h
 *
 *  Created on: May 2, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_THREADENGINE_H_
#define INCLUDE_PHY_THREADENGINE_H_

#include "Engine.h"
#include "Stream.h"


namespace vnl { namespace phy {

class ThreadEngine : public Engine {
protected:
	
	virtual void send_impl(Message* msg, Node* dst, bool async) override {
		assert(msg->isack == false);
		assert(dst);
		
		dst->receive(msg);
		pending++;
		if(!async && pending > 0) {
			wait_for_ack(msg);
		}
	}
	
	virtual bool poll(Stream* stream, int64_t micros) override {
		assert(stream);
		assert(stream->getEngine() == this);
		
		if(micros != 0) {
			return timed_poll(stream, micros);
		} else {
			return inf_poll(stream);
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
	
	virtual void fork(Object* object) override {
		std::thread thread(std::bind(&ThreadEngine::entry, object));
		thread.detach();
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
	
	bool inf_poll(Stream* stream) {
		while(true) {
			Message* msg = collect(-1);
			if(msg) {
				handle(msg);
				if(msg->dst == stream && !msg->isack) {
					return true;
				}
			} else {
				return false;
			}
		}
	}
	
	bool timed_poll(Stream* stream, int64_t micros) {
		int64_t now = System::currentTimeMicros();
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
				return false;
			}
			now = System::currentTimeMicros();
		}
	}
	
	void handle(Message* msg) {
		if(msg->isack) {
			if(msg->callback) {
				msg->callback(msg);
			}
			pending--;
		} else {
			msg->dst->receive(msg);
		}
	}
	
	static void entry(Object* object) {
		ThreadEngine engine;
		engine.exec(object);
	}
	
private:
	int pending = 0;
	
	
};



}}

#endif /* INCLUDE_PHY_THREADENGINE_H_ */
