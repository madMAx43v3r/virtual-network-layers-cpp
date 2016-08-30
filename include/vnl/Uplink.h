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
		:	UplinkBase(domain_, topic_), fd(-1), out(&sock), timer(0), next_seq(1)
	{
		sub_topic = Address("vnl/downlink", "subscribe");
	}
	
protected:
	virtual void main() {
		timer = set_timeout(0, std::bind(&Uplink::write_out, this), VNL_TIMER_MANUAL);
		run();
		for(Address& topic : table.keys()) {
			Super::unsubscribe(topic);
		}
		drop_all();
	}
	
	virtual void reset() {
		out.reset();
		sock = vnl::io::Socket(fd);
		for(Topic& topic : table.values()) {
			do_subscribe(topic);
		}
	}
	
	virtual void publish(const vnl::String& domain, const vnl::String& topic) {
		vnl::Topic desc;
		desc.domain = domain;
		desc.name = topic;
		publish(desc);
	}
	
	virtual void publish(const vnl::Topic& topic) {
		Super::subscribe(topic.domain, topic.name);
		log(INFO).out << "Publishing " << topic.domain << ":" << topic.name << vnl::endl;
	}
	
	virtual void subscribe(const vnl::String& domain, const vnl::String& topic) {
		vnl::Topic desc;
		desc.domain = domain;
		desc.name = topic;
		subscribe(desc);
	}
	
	virtual void subscribe(const vnl::Topic& topic) {
		do_subscribe(topic);
		table[Address(topic.domain, topic.name)] = topic;
	}
	
	virtual void forward(int64_t domain, int64_t topic) {
		Super::subscribe(Address((uint64_t)domain, (uint64_t)topic));
		log(INFO).out << "Forwarding " << vnl::hex(domain) << ":" << vnl::hex(topic) << vnl::endl;
	}
	
	virtual bool handle(Packet* pkt) {
		if(UplinkBase::handle(pkt)) {
			return true;
		}
		if(pkt->pkt_id == vnl::Sample::PID || pkt->pkt_id == vnl::Frame::PID) {
			if(pkt->dst_addr != my_address) {
				queue.push(pkt);
				timer->reset();
				return true;
			}
		}
		return false;
	}
	
	void write_out() {
		int64_t begin = vnl::currentTimeMillis();
		Packet* pkt = 0;
		while(queue.pop(pkt)) {
			write(pkt);
			pkt->ack();
			if(send_timeout >= 0) {
				int64_t now = vnl::currentTimeMillis();
				if(now - begin > send_timeout) {
					break;
				}
			}
		}
		drop_all(); // drop the rest
	}
	
	void drop_all() {
		Packet* pkt = 0;
		while(queue.pop(pkt)) {
			num_drop++;
			pkt->ack();
		}
	}
	
	void write(Packet* pkt) {
		pkt->serialize(out);
		out.flush();
	}
	
	void do_subscribe(const vnl::Topic& topic) {
		Sample sample;
		sample.seq_num = next_seq++;
		sample.src_addr = my_address;
		sample.dst_addr = sub_topic;
		sample.data = topic.clone();
		write(&sample);
		log(INFO).out << "Subscribed to " << topic.domain << ":" << topic.name << vnl::endl;
	}
	
protected:
	int fd;
	Address sub_topic;
	vnl::io::Socket sock;
	vnl::io::TypeOutput out;
	Timer* timer;
	vnl::Queue<Packet*> queue;
	vnl::Map<Address, Topic> table;
	uint32_t next_seq;
	
};




}

#endif /* INCLUDE_VNI_UPLINK_H_ */
