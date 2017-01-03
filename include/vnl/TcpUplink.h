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
#include <vnl/ThreadEngine.h>

#include <vnl/TcpUplinkSupport.hxx>
#include <vnl/info/RemoteInfo.hxx>

#include <thread>


namespace vnl {

class TcpUplink : public vnl::TcpUplinkBase {
public:
	TcpUplink(const vnl::String& domain_, const vnl::String& topic_)
		:	TcpUplinkBase(domain_, topic_),
		 	out(&sock), timer(0), next_seq(1), pipe(0), do_reset(false)
	{
		remote_domain = "vnl.remote";
	}
	
	typedef MessageType<Address, 0xc94ccb93> forward_t;
	typedef MessageType<int, 0x283a6425> error_t;
	
protected:
	virtual int connect() = 0;
	
	void main(Engine* engine, Message* init) {
		init->ack();
		add_input(tunnel);
		timer = set_timeout(0, std::bind(&TcpUplink::write_out, this), VNL_TIMER_MANUAL);
		while(vnl_dorun) {
			sock.fd = connect();
			if(!sock.good()) {
				exit();
				break;
			}
			out.reset();
			do_reset = false;
			add_input(downlink);
			write_announce();
			for(Topic& topic : table.values()) {
				write_subscribe(topic.domain, topic.name);
			}
			pipe = Pipe::create(&downlink);
			std::thread thread(std::bind(&TcpUplink::read_loop, this));
			are_connected = true;
			while(poll(-1)) {
				if(do_reset) {
					break;
				}
			}
			are_connected = false;
			pipe->close();
			downlink.close();
			::shutdown(sock.fd, SHUT_RDWR);		// make read_loop() exit
			thread.join();
			sock.close();
			if(do_reset) {
				usleep(error_interval);
			}
		}
		tunnel.close();
		drop_all();
	}
	
	bool handle(Message* msg) {
		if(Super::handle(msg)) {
			return true;
		}
		if(msg->msg_id == forward_t::MID) {
			Address& addr = ((forward_t*)msg)->data;
			tunnel.subscribe(addr);
			log(DEBUG).out << "Forwarding " << addr << vnl::endl;
		} else if(msg->msg_id == error_t::MID) {
			int err = ((error_t*)msg)->data;
			if(err == VNL_IO_EOF) {
				log(INFO).out << "Connection reset." << vnl::endl;
			} else {
				log(ERROR).out << "Invalid input data: error=" << err << vnl::endl;
			}
			reset();
		}
		return false;
	}
	
	bool handle(Packet* pkt) {
		if(get_channel() == &tunnel) {
			if(sock.good()) {
				queue.push(pkt);
				timer->reset();
			} else {
				pkt->ack();
			}
			return true;
		}
		if(Super::handle(pkt)) {
			return true;
		}
		return false;
	}
	
	void publish(const vnl::String& domain, const vnl::String& topic) {
		tunnel.subscribe(Address(domain, topic));
		log(INFO).out << "Publishing " << domain << ":" << topic << vnl::endl;
	}
	
	void unpublish(const vnl::String& domain, const vnl::String& topic) {
		tunnel.unsubscribe(Address(domain, topic));
		log(INFO).out << "Unpublishing " << domain << ":" << topic << vnl::endl;
	}
	
	// TODO: make this recursive
	void subscribe(const vnl::String& domain, const vnl::String& topic) {
		if(sock.good()) {
			write_subscribe(domain, topic);
		}
		vnl::Topic& data = table[Address(domain, topic)];
		data.domain = domain;
		data.name = topic;
	}
	
	// TODO: make this recursive
	void unsubscribe(const vnl::String& domain, const vnl::String& topic) {
		if(sock.good()) {
			write_subscribe(domain, topic, true);
		}
		table.erase(Address(domain, topic));
	}
	
	vnl::info::RemoteInfo get_remote_info() const {
		return remote_info;
	}
	
	void reset() {
		do_reset = true;
	}
	
	void handle(const vnl::info::RemoteInfo& remote) {
		remote_info = remote;
		Object::publish(remote.clone(), my_private_domain, "remote_info");
	}
	
	void handle(const vnl::Topic& topic, const vnl::Packet& packet) {
		if(packet.dst_addr.topic() == "subscribe") {
			publish(topic.domain, topic.name);
		} else if(packet.dst_addr.topic() == "unsubscribe") {
			unpublish(topic.domain, topic.name);
		}
	}
	
private:
	void write_out() {
		Packet* pkt = 0;
		while(queue.pop(pkt)) {
			pkt->serialize(out);
			pkt->ack();
			num_write++;
			if(out.error()) {
				break;
			}
		}
		out.flush();
		num_flush++;
		if(out.error()) {
			reset();
		}
		drop_all(); // drop the rest
	}
	
	void drop_all() {
		Packet* pkt = 0;
		while(queue.pop(pkt)) {
			pkt->ack();
		}
	}
	
	void write_announce() {
		Sample sample;
		sample.seq_num = next_seq++;
		sample.src_mac = get_mac();
		sample.src_addr = my_address;
		sample.dst_addr = Address(remote_domain, "announce");
		vnl::info::RemoteInfo* info = vnl::info::RemoteInfo::create();
		info->domain_name = vnl::local_domain_name;
		info->config_name = vnl::local_config_name;
		sample.data = info;
		sample.serialize(out);
		out.flush();
	}
	
	void write_subscribe(const vnl::String& domain, const vnl::String& topic, bool do_negate = false) {
		Sample sample;
		sample.seq_num = next_seq++;
		sample.src_mac = get_mac();
		sample.src_addr = my_address;
		sample.dst_addr = Address(remote_domain, do_negate ? "unsubscribe" : "subscribe");
		vnl::Topic* data = vnl::Topic::create();
		data->domain = domain;
		data->name = topic;
		sample.data = data;
		sample.serialize(out);
		out.flush();
		if(do_negate) {
			log(INFO).out << "Unsubscribed from " << domain << ":" << topic << vnl::endl;
		} else {
			log(INFO).out << "Subscribed to " << domain << ":" << topic << vnl::endl;
		}
	}
	
private:
	void read_loop() {
		ThreadEngine engine;
		Stream stream;
		stream.connect(&engine);
		stream.set_timeout(vnl_msg_timeout);
		vnl::io::TypeInput in(&sock);
		in.reset();
		pipe->attach();
		while(vnl_dorun) {
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
				if(!read_packet(stream, in, hash)) {
					error = true;
				}
				num_read++;
			} else {
				error = true;
			}
			if(error) {
				if(vnl_dorun) {
					error_t msg(in.error());
					stream.send(&msg, pipe, true);
				}
				break;
			}
		}
		pipe->detach();
		stream.close();
		engine.flush();
	}
	
	bool read_packet(Stream& stream, vnl::io::TypeInput& in, uint32_t hash) {
		if(hash == Sample::PID) {
			Sample* sample = sample_buffer.create();
			sample->deserialize(in, 0);
			sample->proxy = get_mac();
			if(!in.error()) {
				if(sample->dst_addr.domain() == remote_domain) {
					stream.send_async(sample, pipe, true);
				} else {
					forward(stream, sample);
					stream.send_async(sample, sample->dst_addr);
				}
				return true;
			}
			sample->destroy();
		} else if(hash == Frame::PID) {
			Frame* frame = frame_buffer.create();
			frame->deserialize(in, 0);
			frame->proxy = get_mac();
			if(!in.error()) {
				if(frame->dst_addr.domain() == remote_domain) {
					stream.send_async(frame, pipe, true);
				} else {
					forward(stream, frame);
					stream.send_async(frame, frame->dst_addr, frame->type == Frame::RESULT);
				}
				return true;
			}
			frame->destroy();
		}
		return false;
	}
	
	void forward(Stream& stream, vnl::Packet* pkt) {
		int& count = fwd_table[pkt->src_addr];
		if(count == 0) {
			forward_t msg(pkt->src_addr);
			stream.send(&msg, pipe, true);
		}
		count++;
	}
	
private:
	MessagePool<Sample> sample_buffer;
	MessagePool<Frame> frame_buffer;
	
private:
	Hash64 remote_domain;
	vnl::info::RemoteInfo remote_info;
	
	vnl::io::Socket sock;
	vnl::io::TypeOutput out;
	Timer* timer;
	Stream tunnel;
	Stream downlink;
	vnl::Queue<Packet*> queue;
	vnl::Map<Address, Topic> table;
	uint32_t next_seq;
	Pipe* pipe;
	bool do_reset;
	
	Map<Address, int> fwd_table;
	
};


} // vnl

#endif /* INCLUDE_VNL_TCPUPLINK_H_ */
