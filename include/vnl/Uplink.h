/*
 * Uplink.h
 *
 *  Created on: Jul 1, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNI_UPLINK_H_
#define INCLUDE_VNI_UPLINK_H_

#include <vnl/Module.h>
#include <vnl/Map.h>
#include <vnl/io/Socket.h>


namespace vnl {

class Uplink : public vnl::Module {
public:
	Uplink(const vnl::String& name)
		:	Module(name),
			enable(0), out(0)
	{
		my_addr = vnl::Address(vnl::get_local_domain(), get_mac());
	}
	
	typedef vnl::MessageType<vnl::Address, 0x6245e4cc> open_t;
	typedef vnl::MessageType<vnl::Address, 0xc1d15cc6> close_t;
	
	typedef vnl::MessageType<vnl::io::Socket*, 0x3273cc98> enable_t;
	
protected:
	virtual void main(vnl::Engine* engine) {
		open(my_addr);
		run();
		if(enable) {
			enable->ack();
		}
		close(my_addr);
	}
	
	virtual bool handle(vnl::Message* msg) {
		if(msg->msg_id == enable_t::MID) {
			if(enable) {
				enable->ack();
			}
			enable = (enable_t*)msg;
			out = vnl::io::TypeOutput(enable->data);
			for(vnl::Address addr : table.keys()) {
				out.putEntry(VNL_IO_INTERFACE, VNL_IO_BEGIN);
				out.putHash(open_t::MID);
				addr.serialize(out);
				out.putEntry(VNL_IO_INTERFACE, VNL_IO_END);
			}
			out.flush();
			log(DEBUG).out << "Enabled on socket " << vnl::dec(enable->data->get_fd()) << vnl::endl;
		} else if(enable) {
			if(msg->msg_id == open_t::MID) {
				vnl::Address addr = ((open_t*)msg)->data;
				out.putEntry(VNL_IO_INTERFACE, VNL_IO_BEGIN);
				out.putHash(open_t::MID);
				addr.serialize(out);
				out.putEntry(VNL_IO_INTERFACE, VNL_IO_END);
				out.flush();
				table[addr] = 1;
			} else if(msg->msg_id == close_t::MID) {
				vnl::Address addr = ((close_t*)msg)->data;
				out.putEntry(VNL_IO_INTERFACE, VNL_IO_BEGIN);
				out.putHash(close_t::MID);
				addr.serialize(out);
				out.putEntry(VNL_IO_INTERFACE, VNL_IO_END);
				out.flush();
				table.erase(addr);
			}
		}
		return Module::handle(msg);
	}
	
	virtual bool handle(vnl::Packet* pkt) {
		if(pkt->pkt_id == vni::PID_SAMPLE || pkt->pkt_id == vni::PID_FRAME) {
			if(enable) {
				pkt->serialize(out);
				out.flush();
			}
		}
		return false;
	}
	
private:
	vnl::Address my_addr;
	enable_t* enable;
	vnl::io::TypeOutput out;
	vnl::Map<vnl::Address, int> table;
	
};




}

#endif /* INCLUDE_VNI_UPLINK_H_ */
