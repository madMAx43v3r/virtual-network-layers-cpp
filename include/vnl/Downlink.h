/*
 * Downlink.h
 *
 *  Created on: Jul 3, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_DOWNLINK_H_
#define CPP_INCLUDE_VNI_DOWNLINK_H_

#include <vnl/Map.h>
#include <vnl/Sample.h>
#include <vnl/UplinkSupport.hxx>
#include <vnl/DownlinkSupport.hxx>


namespace vnl {

class Downlink : public vnl::DownlinkBase {
public:
	Downlink(const vnl::String& domain_, const vnl::String& topic_)
		:	DownlinkBase(domain_, topic_)
	{
		sub_topic = Address("vnl/downlink", "subscribe");
	}
	
	virtual void receive(vnl::Message* msg) {
		if(msg->msg_id == close_t::MID) {
			dorun = false;
			sock.close();
		}
		Super::receive(msg);
	}
	
	UplinkClient uplink;
	
	typedef vnl::SignalType<0x2965956f> close_t;
	
protected:
	virtual void main(vnl::Engine* engine) {
		uplink.connect(engine);
		while(dorun) {
			int fd = -1;
			uplink.reset();
			uplink.get_fd(fd);
			if(fd < 0) {
				break;
			}
			sock = vnl::io::Socket(fd);
			vnl::io::TypeInput in(&sock);
			while(dorun) {
				int size = 0;
				int id = in.getEntry(size);
				if(id == VNL_IO_INTERFACE && size == VNL_IO_BEGIN) {
					uint32_t hash = 0;
					in.getHash(hash);
					if(!read_packet(in, hash)) {
						log(ERROR).out << "Invalid input data: hash=" << vnl::hex(hash) << vnl::endl;
						break;
					}
				} else {
					log(ERROR).out << "Invalid input data: id=" << id << " size=" << size << vnl::endl;
					break;
				}
				poll(0);
			}
		}
		if(!Layer::shutdown) {
			uplink.shutdown();
		}
		// wait for close signal
		run();
	}
	
	bool read_packet(vnl::io::TypeInput& in, uint32_t hash) {
		switch(hash) {
		case Sample::PID: if(do_deserialize) {
			Sample* sample = buffer.create<Sample>();
			sample->deserialize(in, 0);
			if(sample->data && !in.error()) {
				if(sample->dst_addr == sub_topic) {
					Topic* topic = dynamic_cast<Topic*>(sample->data);
					if(topic) {
						uplink.publish(*topic);
					}
					sample->ack();
				} else {
					forward(sample);
					send_async(sample, sample->dst_addr);
				}
			} else {
				if(sample->data) {
					log(ERROR).out << "Invalid Sample: " << sample->data->type_name() << vnl::endl;
				} else {
					log(ERROR).out << "Invalid Sample: <unknown>" << vnl::endl;
				}
				sample->ack();
			}
		} else {
			BinarySample* sample = buffer.create<BinarySample>();
			sample->deserialize(in, 0);
			if(sample->data && !in.error()) {
				forward(sample);
				send_async(sample, sample->dst_addr);
			} else {
				log(ERROR).out << "Invalid BinarySample: size=" << sample->size << vnl::endl;
				sample->ack();
			}
		}
		break;
		case Frame::PID: {
			Frame* frame = buffer.create<Frame>();
			frame->deserialize(in, 0);
			if(!in.error()) {
				forward(frame);
				send_async(frame, frame->dst_addr);
			} else {
				frame->ack();
			}
			break;
		}
		default: return false;
		}
		return true;
	}
	
	void forward(vnl::Packet* pkt) {
		int& count = fwd_table[pkt->src_addr];
		if(count == 0) {
			uplink.forward(pkt->src_addr.A, pkt->src_addr.B);
		}
		count++;
	}
	
private:
	vnl::io::Socket sock;
	Address sub_topic;
	Map<Address, int> fwd_table;
	
};



}

#endif /* CPP_INCLUDE_VNI_DOWNLINK_H_ */
