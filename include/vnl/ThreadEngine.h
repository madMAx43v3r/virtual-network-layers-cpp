/*
 * ThreadEngine.h
 *
 *  Created on: May 2, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_THREADENGINE_H_
#define INCLUDE_PHY_THREADENGINE_H_

#include <thread>
#include <vnl/Engine.h>
#include <vnl/Stream.h>
#include <vnl/Object.h>


namespace vnl {

class ThreadEngine : public Engine {
public:
	virtual void fork(Object* object) {
		spawn(object);
	}
	
	virtual void run(Object* object) {
		Message msg;
		exec(object, &msg);
	}
	
	static void spawn(Object* object) {
		Actor sync;
		Message msg;
		msg.src = &sync;
		std::thread thread(&ThreadEngine::entry, object, &msg);
		thread.detach();
		sync.wait();
	}
	
protected:
	virtual void send_impl(Message* msg, Basic* dst, bool async) {
		assert(msg->isack == false);
		assert(dst);
		
		msg->src = this;
		dst->receive(msg);
		pending++;
		if(!async && pending > 0) {
			wait_for_ack(msg);
		}
	}
	
	virtual bool poll(Stream* stream, int64_t micros) {
		assert(stream);
		assert(stream->get_engine() == this);
		
		if(micros > 0) {
			return timed_poll(stream, micros);
		} else {
			return simple_poll(stream, micros);
		}
	}
	
	virtual void flush() {
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
			msg->destroy();
			pending--;
		} else {
			msg->dst->receive(msg);
		}
	}
	
	static void entry(Object* object, Message* msg) {
		ThreadEngine engine;
		engine.exec(object, msg);
	}
	
private:
	int pending = 0;
	
};


inline void spawn(Object* object) {
	ThreadEngine::spawn(object);
}

inline void run(Object* object) {
	ThreadEngine engine;
	engine.run(object);
}



}

#endif /* INCLUDE_PHY_THREADENGINE_H_ */
