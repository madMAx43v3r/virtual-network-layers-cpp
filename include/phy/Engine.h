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

#include "Message.h"
#include "Stream.h"

namespace vnl { namespace phy {

class Engine {
public:
	Engine();
	virtual ~Engine() {}
	
	virtual void send(Message* msg) = 0;
	virtual void flush() = 0;
	virtual void handle(Message* msg) = 0;
	virtual void open(Stream* stream) = 0;
	virtual void close(Stream* stream) = 0;
	virtual bool poll(Stream* stream, int millis) = 0;
	virtual uint64_t launch(Runnable* task) = 0;
	virtual void cancel(uint64_t tid) = 0;
	virtual int timeout() = 0;
	
	uint64_t rand();
	
protected:
	void forward(Message* msg);
	void dispatch(Message* msg);
	void dispatch(Message* msg, Stream* stream);
	Stream* get_stream(Object* obj, uint64_t sid);
	
private:
	std::default_random_engine generator;
	
};


Engine::Engine() {
	static std::atomic<int> counter;
	generator.seed(Util::hash64(counter++, System::nanoTime()));
}

uint64_t Engine::rand() {
	return Util::hash64(generator());
}


}}

#endif /* INCLUDE_PHY_ENGINE_H_ */
