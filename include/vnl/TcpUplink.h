/*
 * TcpUplink.h
 *
 *  Created on: Jul 1, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_TCPUPLINK_H_
#define INCLUDE_VNL_TCPUPLINK_H_

#include <vnl/Sample.h>
#include <vnl/Pipe.h>
#include <vnl/io/Socket.h>
#include <vnl/TcpUplinkSupport.hxx>
#include <vnl/ThreadEngine.h>

#include <thread>


namespace vnl {

class TcpUplink : public vnl::TcpUplinkBase {
public:
	TcpUplink(const vnl::String& domain_, const vnl::String& topic_)
		:	TcpUplinkBase(domain_, topic_), out(&sock), timer(0), next_seq(1), do_reset(false)
	{
		sub_topic = Address("vnl.downlink", "subscribe");
	}
	
	typedef MessageType<Topic, 0x2b70291f> subscribe_t;
	typedef MessageType<Address, 0xc94ccb93> forward_t;
	typedef MessageType<int, 0x283a6425> error_t;
	
protected:
	virtual int connect() = 0;
	
	void main() {
		timer = set_timeout(0, std::bind(&TcpUplink::write_out, this), VNL_TIMER_MANUAL);
		while(dorun) {
			are_connected = false;
			sock.fd = connect();
			if(!sock.good()) {
				exit();
				break;
			}
			are_connected = true;
			out.reset();
			for(Topic& topic : table.values()) {
				write_subscribe(topic);
			}
			attach(&pipe);		// first attach pipe, then start thread
			std::thread thread(std::bind(&TcpUplink::read_loop, this));
			while(poll(-1)) {
				if(do_reset) {
					do_reset = false;
					usleep(error_interval);
					break;
				}
			}
			close(&pipe);		// stop read_loop() from sending us messages
			poll(0);			// handle remaining messages
			::shutdown(sock.fd, SHUT_RDWR);		// make read_loop() exit in any case
			thread.join();		// join with read_loop()
			sock.close();
		}
		drop_all();
	}
	
	bool handle(Message* msg) {
		if(Super::handle(msg)) {
			return true;
		}
		if(msg->msg_id == subscribe_t::MID) {
			publish(((subscribe_t*)msg)->data);
		} else if(msg->msg_id == forward_t::MID) {
			Address& addr = ((forward_t*)msg)->data;
			Object::subscribe(addr);
			log(DEBUG).out << "Forwarding " << addr << vnl::endl;
		} else if(msg->msg_id == error_t::MID) {
			int err = ((error_t*)msg)->data;
			if(err != VNL_IO_EOF) {
				log(ERROR).out << "Invalid input data: error=" << err << vnl::endl;
			}
			do_reset = true;
		}
		return false;
	}
	
	bool handle(Packet* pkt) {
		if(Super::handle(pkt)) {
			return true;
		}
		if(sock.good()) {
			queue.push(pkt);
			timer->reset();
			return true;
		}
		return false;
	}
	
	void publish(const vnl::String& domain, const vnl::String& topic) {
		vnl::Topic desc;
		desc.domain = domain;
		desc.name = topic;
		publish(desc);
	}
	
	void publish(const vnl::Topic& topic) {
		Object::subscribe(topic.domain, topic.name);
		log(INFO).out << "Publishing " << topic.domain << ":" << topic.name << vnl::endl;
	}
	
	void subscribe(const vnl::String& domain, const vnl::String& topic) {
		vnl::Topic desc;
		desc.domain = domain;
		desc.name = topic;
		subscribe(desc);
	}
	
	void subscribe(const vnl::Topic& topic) {
		if(sock.good()) {
			write_subscribe(topic);
		}
		table[Address(topic.domain, topic.name)] = topic;
	}
	
	void write_out() {
		int64_t begin = vnl::currentTimeMicros();
		Packet* pkt = 0;
		while(queue.pop(pkt)) {
			pkt->serialize(out);
			pkt->ack();
			if(out.error()) {
				break;
			}
			if(send_timeout >= 0) {
				int64_t now = vnl::currentTimeMicros();
				if(now - begin > send_timeout) {
					break;
				}
			}
		}
		out.flush();
		if(out.error()) {
			do_reset = true;
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
	
	void write_subscribe(const vnl::Topic& topic) {
		Sample sample;
		sample.seq_num = next_seq++;
		sample.src_mac = mac;
		sample.src_addr = my_address;
		sample.dst_addr = sub_topic;
		sample.data = topic.clone();
		sample.serialize(out);
		out.flush();
		log(INFO).out << "Subscribed to " << topic.domain << ":" << topic.name << vnl::endl;
	}
	
protected:
	void read_loop() {
		ThreadEngine engine;
		MessagePool buffer;
		Stream stream;
		stream.connect(&engine);
		vnl::io::TypeInput in(&sock);
		in.reset();
		while(dorun) {
			while(true) {
				Message* msg = stream.poll(0);
				if(!msg) {
					break;
				}
				msg->ack();
			}
			bool error = false;
			int size = 0;
			int id = in.getEntry(size);
			if(!in.error() && id == VNL_IO_INTERFACE && size == VNL_IO_BEGIN) {
				uint32_t hash = 0;
				in.getHash(hash);
				if(!read_packet(stream, buffer, in, hash)) {
					error = true;
				}
			} else {
				error = true;
			}
			if(error) {
				if(dorun) {
					error_t msg(in.error());
					stream.send(&msg, &pipe);
				}
				break;
			}
		}
		engine.flush();
	}
	
	bool read_packet(Stream& stream, MessagePool& buffer, vnl::io::TypeInput& in, uint32_t hash) {
		if(hash == Sample::PID) {
			Sample* sample = buffer.create<Sample>();
			sample->deserialize(in, 0);
			if(!in.error()) {
				if(sample->dst_addr == sub_topic) {
					Topic* topic = dynamic_cast<Topic*>(sample->data);
					if(topic) {
						subscribe_t* msg = buffer.create<subscribe_t>();
						msg->data = *topic;
						stream.send_async(msg, &pipe);
					}
					sample->destroy();
				} else {
					forward(stream, buffer, sample);
					stream.send_async(sample, sample->dst_addr);
				}
			} else {
				sample->destroy();
				return false;
			}
		} else if(hash == Frame::PID) {
			Frame* frame = buffer.create<Frame>();
			frame->deserialize(in, 0);
			if(!in.error()) {
				forward(stream, buffer, frame);
				stream.send_async(frame, frame->dst_addr);
			} else {
				frame->destroy();
				return false;
			}
		} else {
			return false;
		}
		return true;
	}
	
	void forward(Stream& stream, MessagePool& buffer, vnl::Packet* pkt) {
		int& count = fwd_table[pkt->src_addr];
		if(count == 0) {
			forward_t* msg = buffer.create<forward_t>();
			msg->data = pkt->src_addr;
			stream.send_async(msg, &pipe);
		}
		count++;
	}
	
private:
	Address sub_topic;
	vnl::io::Socket sock;
	vnl::io::TypeOutput out;
	Timer* timer;
	vnl::Queue<Packet*> queue;
	vnl::Map<Address, Topic> table;
	uint32_t next_seq;
	Pipe pipe;
	bool do_reset;
	
	Map<Address, int> fwd_table;
	
};




}

#endif /* INCLUDE_VNL_TCPUPLINK_H_ */
