/*
 * Uplink.h
 *
 *  Created on: Jul 1, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNI_UPLINK_H_
#define INCLUDE_VNI_UPLINK_H_

#include <vnl/Sample.h>
#include <vnl/io/Socket.h>
#include <vnl/UplinkSupport.hxx>


namespace vnl {

class Uplink : public vnl::UplinkBase {
public:
	Uplink(const vnl::String& domain_, const vnl::String& topic_)
		:	UplinkBase(domain_, topic_), out(&sock), next_seq(1)
	{
		sub_topic = Address("vnl/downlink", "subscribe");
	}
	
protected:
	virtual void main(vnl::Engine* engine, vnl::Message* init) {
		init->ack();
		run();
	}
	
	virtual void reset() {
		for(Topic& topic : table) {
			subscribe(topic);
		}
	}
	
	virtual void publish(const vnl::String& domain, const vnl::String& topic) {
		vnl::Topic desc;
		desc.domain = domain;
		desc.name = topic;
		publish(desc);
	}
	
	virtual void publish(const vnl::Topic& topic) {
		open(topic.domain, topic.name);
		log(INFO).out << "Publishing " << topic.domain << ":" << topic.name << vnl::endl;
	}
	
	virtual void subscribe(const vnl::String& domain, const vnl::String& topic) {
		vnl::Topic desc;
		desc.domain = domain;
		desc.name = topic;
		subscribe(desc);
	}
	
	virtual void subscribe(const vnl::Topic& topic) {
		Topic tmp = topic;
		Sample sample;
		sample.seq_num = next_seq++;
		sample.src_addr = my_address;
		sample.dst_addr = sub_topic;
		sample.data = &tmp;
		write(&sample);
		sample.data = 0;
		table.push_back(topic);
		log(INFO).out << "Subscribed to " << topic.domain << ":" << topic.name << vnl::endl;
	}
	
	virtual void forward(int64_t domain, int64_t topic) {
		open((uint64_t)domain, (uint64_t)topic);
	}
	
	virtual bool handle(vnl::Packet* pkt) {
		if(UplinkBase::handle(pkt)) {
			return true;
		}
		if(pkt->pkt_id == vnl::Sample::PID || pkt->pkt_id == vnl::Frame::PID) {
			if(pkt->dst_addr != my_address) {
				write(pkt);
			}
		}
		return false;
	}
	
	void write(vnl::Packet* pkt) {
		pkt->serialize(out);
		out.flush();
	}
	
protected:
	Address sub_topic;
	vnl::io::Socket sock;
	vnl::io::TypeOutput out;
	vnl::List<Topic> table;
	uint32_t next_seq;
	
};




}

#endif /* INCLUDE_VNI_UPLINK_H_ */
