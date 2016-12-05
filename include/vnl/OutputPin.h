/*
 * OutputPin.h
 *
 *  Created on: Nov 30, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_OUTPUTPIN_H_
#define INCLUDE_VNL_OUTPUTPIN_H_

#include <vnl/Pipe.h>
#include <vnl/Pool.h>
#include <vnl/List.h>
#include <vnl/Value.hxx>


namespace vnl {

class OutputPin {
public:
	OutputPin() : enabled(false), engine(0) {}
	
	~OutputPin() {
		if(enabled) {
			close();
		}
	}
	
	class pin_data_t : public MessageType<const Value*, 0xe8a3ef08> {
	public:
		pin_data_t() : parent(0), count(0), acks(0) {}
		~pin_data_t() {
			if(parent) {
				if(++(parent->acks) == parent->count) {
					vnl::destroy(parent->data);
					parent->destroy();
				}
			}
		}
		pin_data_t* parent;
		int count;
		int acks;
	};
	
	// NOT thread safe
	Pipe* attach() {
		assert(enabled == false);
		Pipe* pipe = pipe_pool.create();
		links.push_back(pipe);
		return pipe;
	}
	
	// NOT thread safe
	void enable(Engine* engine) {
		this->engine = engine;
		enabled = true;
	}
	
	// NOT thread safe
	void transmit(Value* data) {
		assert(enabled == true);
		pin_data_t* parent = buffer.create();
		parent->data = data;
		for(Pipe* pipe : links) {
			pin_data_t* msg = buffer.create();
			msg->parent = parent;
			msg->data = data;
			engine->send_async(msg, pipe);
			parent->count++;
		}
	}
	
protected:
	void close() {
		assert(enabled == true);
		MessagePool<Pipe::close_t> buf;
		for(Pipe* pipe : links) {
			Pipe::close_t* msg = buf.create();
			engine->send_async(msg, pipe);
		}
		engine->flush();
		for(Pipe* pipe : links) {
			pipe_pool.destroy(pipe);
		}
		links.clear();
	}
	
private:
	bool enabled;
	Engine* engine;
	MessagePool<pin_data_t> buffer;
	Pool<Pipe> pipe_pool;
	List<Pipe*> links;
	
};


} // vnl

#endif /* INCLUDE_VNL_OUTPUTPIN_H_ */
