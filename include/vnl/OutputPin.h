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

class OutputPin : public Node {
public:
	OutputPin(const char* name = "OutputPin") : name(name), enabled(false), engine(0) {}
	
	~OutputPin() {
		assert(enabled == false);
	}
	
	class pin_data_t : public MessageType<Value*, 0xe8a3ef08> {
	public:
		pin_data_t() : parent(0), count(0), acks(0) {}
		~pin_data_t() {
			if(parent) {
				if(++(parent->acks) == parent->count) {
					parent->destroy();
				}
			} else {
				vnl::destroy(data);
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
	void transmit(Value* data, bool no_drop = false) {
		assert(enabled == true);
		if(links.empty()) {
			vnl::destroy(data);
			return;
		}
		pin_data_t* parent = buffer.create();
		parent->data = data;
		for(Pipe* pipe : links) {
			pin_data_t* msg = buffer.create();
			msg->src_mac = vnl_mac;
			msg->parent = parent;
			msg->data = data;
			msg->timeout = send_timeout;
			msg->is_no_drop = no_drop;
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
	
	// NOT thread safe
	void set_timeout(int64_t to_usec) {
		send_timeout = to_usec;
	}
	
	String name;
	
private:
	bool enabled;
	Engine* engine;
	MessagePool<pin_data_t> buffer;
	List<Pipe*> links;
	int64_t send_timeout = 1000000;
	
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
