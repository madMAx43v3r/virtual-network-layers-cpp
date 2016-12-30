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
#include <vnl/Type.hxx>


namespace vnl {

class OutputPin {
public:
	OutputPin(const char* name = "Output") : name(name), enabled(false), engine(0) {}
	
	~OutputPin() {
		assert(enabled == false);
	}
	
	class pin_data_t : public MessageType<Value*, 0xe8a3ef08> {
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
	Pipe* create() {
		assert(enabled == false);
		Pipe* pipe = Pipe::create();
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
		if(links.empty()) {
			vnl::destroy(data);
			return;
		}
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
	
	// NOT thread safe
	void close() {
		for(Pipe* pipe : links) {
			pipe->close();
		}
		links.clear();
		engine = 0;
		enabled = false;
	}
	
	String name;
	
private:
	bool enabled;
	Engine* engine;
	MessagePool<pin_data_t> buffer;
	List<Pipe*> links;
	
};


template<typename T>
class TypedOutputPin : public OutputPin {
public:
	void transmit(T* data) {
		OutputPin::transmit(data);
	}
	
private:
	void transmit(Value* data) {
		OutputPin::transmit(data);
	}
	
};


} // vnl

#endif /* INCLUDE_VNL_OUTPUTPIN_H_ */
