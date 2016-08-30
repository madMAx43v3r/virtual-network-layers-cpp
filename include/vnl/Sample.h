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


class BinarySample : public vnl::Packet {
public:
	static const uint32_t PID = 0xfd63c06a;
	
	BinarySample() : data(0), size(0) {
		pkt_id = PID;
		payload = this;
	}
	
	~BinarySample() {
		if(data) {
			data->free_all();
		}
	}
	
	vnl::Page* data;
	int size;
	
protected:
	virtual void write(vnl::io::TypeOutput& out) const {
		out.putBinary(data, size);
	}
	
	virtual void read(vnl::io::TypeInput& in) {
		if(!data) {
			data = Page::alloc();
		}
		vnl::io::ByteBuffer buf(data);
		vnl::io::TypeOutput out(&buf);
		in.copy(&out);
		size = buf.position();
	}
	
};


}

#endif /* INCLUDE_VNL_SAMPLE_H_ */
