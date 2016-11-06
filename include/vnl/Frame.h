/*
 * Frame.h
 *
 *  Created on: Jun 30, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNI_FRAME_H_
#define INCLUDE_VNI_FRAME_H_

#include <vnl/Packet.h>


namespace vnl {

class Frame : public vnl::Packet {
public:
	static const uint32_t PID = 0xde2104f2;
	
	Frame() : data(0), size(0), req_num(0) {
		pkt_id = PID;
		payload = this;
	}
	
	~Frame() {
		if(data) {
			data->free_all();
		}
	}
	
	int64_t req_num;
	vnl::Page* data;
	int size;
	
protected:
	virtual void write(vnl::io::TypeOutput& out) const {
		out.putValue(req_num);
		out.putBinary(data, size);
	}
	
	virtual void read(vnl::io::TypeInput& in) {
		in.getValue(req_num);
		in.getBinary(data, size);
	}
	
};



}

#endif /* INCLUDE_VNI_FRAME_H_ */
