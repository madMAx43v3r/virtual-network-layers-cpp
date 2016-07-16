/*
 * Sample.h
 *
 *  Created on: Jun 30, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_SAMPLE_H_
#define INCLUDE_VNL_SAMPLE_H_

#include <vnl/Packet.h>
#include <vnl/Type.hxx>


namespace vnl {

class Sample : public vnl::Packet {
public:
	static const uint32_t PID = 0xa37b95eb;
	
	Sample() {
		pkt_id = PID;
		payload = this;
	}
	
	~Sample() {
		vnl::destroy(data);
	}
	
	Value* data = 0;
	
protected:
	virtual void write(vnl::io::TypeOutput& out) const {
		vnl::write(out, data);
	}
	
	virtual void read(vnl::io::TypeInput& in) {
		data = vnl::read(in);
	}
	
	
};


}

#endif /* INCLUDE_VNL_SAMPLE_H_ */
