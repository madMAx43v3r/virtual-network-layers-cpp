/*
 * ThreadEngine.h
 *
 *  Created on: May 2, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_THREADENGINE_H_
#define INCLUDE_PHY_THREADENGINE_H_

#include <thread>
#include <pthread.h>

#include <vnl/Layer.h>
#include <vnl/Engine.h>
#include <vnl/Pipe.h>
#include <vnl/Actor.h>
#include <vnl/Stream.h>
#include <vnl/Object.h>
#include <vnl/Router.h>


namespace vnl {

class ThreadEngine : public Engine {
public:
	ThreadEngine() : pending(0) {}
	
	~ThreadEngine() {
		assert(pending == 0);
	}
	
	static void spawn(Object* object, Pipe* pipe = 0) {
		Actor sync;
		Message msg;
		msg.src = &sync;
		std::thread thread(&ThreadEngine::entry, object, &msg, pipe);
		thread.detach();
		sync.wait_for_ack();
	}
	
	// all below NOT thread safe
	
	virtual void run(Object* object, Pipe* pipe = 0) {
		Message msg;
		exec(object, &msg, pipe);
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
		assert(pending == 0);
	}
	
protected:
	void exec(Object* object, Message* init, Pipe* pipe) {
		Engine::exec(object, init, pipe);
	}
	
	virtual void send_impl(Message* msg, bool async) {
		assert(msg->isack == false);
		assert(msg->src);
		assert(msg->dst);
		
		msg->dst->receive(msg);
		pending++;
		if(!async && pending > 0) {
			wait_for_ack(msg);
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
				if(!stream->empty()) {
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
				if(!stream->empty()) {
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
		int64_t now = currentTimeMicros();
		if(msg->isack) {
			if(msg->rcv_time) {
				send_latency_sum += now - msg->rcv_time;
			}
			msg->destroy();
			pending--;
		} else {
			if(msg->rcv_time) {
				receive_latency_sum += now - msg->rcv_time;
			}
			if(!msg->is_no_drop && currentTimeMicros() - msg->rcv_time > msg->timeout) {
				msg->ack();
				num_timeout++;
			} else {
				msg->dst->receive(msg);
			}
		}
	}
	
	static void entry(Object* object, Message* msg, Pipe* pipe) {
		{
			char buf[16];
			object->get_my_topic().to_string(buf, sizeof(buf));
			pthread_setname_np(pthread_self(), buf);
		}
		Layer::num_threads++;
		{
			ThreadEngine engine;
			engine.exec(object, msg, pipe);
		}
		Layer::num_threads--;
	}
	
private:
	int pending;
	
};


inline Address spawn(Object* object, Pipe* pipe) {
	Address addr = object->get_my_private_address();
	ThreadEngine::spawn(object, pipe);
	return addr;
}

inline void run(Object* object) {
	ThreadEngine engine;
	engine.run(object);
}



}

#endif /* INCLUDE_PHY_THREADENGINE_H_ */
