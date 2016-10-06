/*
 * Sample.h
 *
 *  Created on: Jun 30, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_SAMPLE_H_
#define INCLUDE_VNL_SAMPLE_H_

#include <vnl/Packet.h>
#include <vnl/Header.hxx>
#include <vnl/Type.hxx>


namespace vnl {

class Sample : public vnl::Packet {
public:
	static const uint32_t PID = 0xa37b95eb;
	
	Sample()
		:	header(0), data(0)
	{
		pkt_id = PID;
		payload = this;
	}
	
	~Sample() {
		vnl::destroy(header);
		vnl::destroy(data);
	}
	
	Header* header;
	Value* data;
	
protected:
	virtual void write(vnl::io::TypeOutput& out) const {
		if(header) {
			vnl::write(out, header);
		}
		vnl::write(out, data);
	}
	
	virtual void read(vnl::io::TypeInput& in) {
		data = vnl::read(in);
		header = dynamic_cast<Header*>(data);
		if(header) {
			data = vnl::read(in);
		}
	}
	
};


}

#endif /* INCLUDE_VNL_SAMPLE_H_ */
