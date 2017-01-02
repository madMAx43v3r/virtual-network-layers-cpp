/*
 * InputPin.h
 *
 *  Created on: Dec 1, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_INPUTPIN_H_
#define INCLUDE_VNL_INPUTPIN_H_

#include <vnl/Stream.h>
#include <vnl/Pipe.h>
#include <vnl/List.h>
#include <vnl/Value.hxx>


namespace vnl {

class InputPin : public Basic {
public:
	InputPin(const char* name = "Input") : name(name), enabled(false) {}
	
	~InputPin() {
		assert(enabled == false);
	}
	
	bool operator==(InputPin& other) const {
		return this == &other;
	}
	
	bool operator!=(InputPin& other) const {
		return this != &other;
	}
	
	// thread safe
	virtual void receive(Message* msg) {
		stream.receive(msg);
	}
	
	// NOT thread safe
	void attach(Pipe* pipe) {
		assert(enabled == false);
		links.push_back(pipe);
	}
	
	// NOT thread safe
	void enable(Engine* engine, Basic* listener) {
		assert(enabled == false);
		stream.connect(engine, 0);
		stream.listen(listener);
		for(Pipe* pipe : links) {
			pipe->open(this);
		}
		enabled = true;
	}
	
	// NOT thread safe
	void close() {
		for(Pipe* pipe : links) {
			pipe->close();
		}
		stream.close();
		links.clear();
		enabled = false;
	}
	
	String name;
	
private:
	bool enabled;
	Stream stream;
	List<Pipe*> links;
	
};


template<typename T>
class TypedInputPin : public InputPin {
public:
	
};


} // vnl

#endif /* INCLUDE_VNL_INPUTPIN_H_ */
