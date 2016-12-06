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
	InputPin() : enabled(false) {}
	
	~InputPin() {
		for(Pipe* pipe : links) {
			pipe->close();
		}
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
	void enable(Engine* engine, Basic* target) {
		assert(enabled == false);
		stream.connect(engine, 0);
		stream.listen(target);
		for(Pipe* pipe : links) {
			pipe->attach(this);
		}
		enabled = true;
	}
	
	// NOT thread safe
	bool pop(Message*& msg) {
		assert(enabled == true);
		return stream.pop(msg);
	}
	
private:
	bool enabled;
	Stream stream;
	List<Pipe*> links;
	
};


} // vnl

#endif /* INCLUDE_VNL_INPUTPIN_H_ */
