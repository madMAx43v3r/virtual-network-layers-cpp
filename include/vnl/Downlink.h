/*
 * Downlink.h
 *
 *  Created on: Jul 3, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_DOWNLINK_H_
#define CPP_INCLUDE_VNI_DOWNLINK_H_

#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>

#include <vnl/Uplink.h>
#include <vnl/Module.h>
#include <vnl/Map.h>
#include <vnl/io/Socket.h>


namespace vnl {

class Downlink : public vnl::Module {
public:
	Downlink(const vnl::String& port)
		:	Module(vnl::String(port) << "/downlink"),
			port(port), sock(0)
	{
	}
	
	static const int error_interval = 3*1000;
	static const int maintain_interval = 10;
	static const int forward_timeout = 99;
	
	virtual void receive(vnl::Message* msg) {
		if(msg->msg_id == vnl::Registry::exit_t::MID) {
			dorun = false;
			if(sock) {
				sock->close();
			}
		}
		msg->ack();
	}
	
protected:
	virtual void main(vnl::Engine* engine) {
		fork(new Uplink(vnl::String(port) << "/uplink"));
		int64_t last_maintain = vnl::currentTime();
		while(dorun) {
			sock = connect();
			if(!sock) {
				break;
			}
			Uplink::enable_t* enable = buffer.create<Uplink::enable_t>();
			enable->data = sock;
			send_async(enable, uplink);
			vnl::io::TypeInput in(sock);
			while(dorun) {
				int size = 0;
				int id = in.getEntry(size);
				if(id == VNL_IO_INTERFACE && size == VNL_IO_BEGIN) {
					uint32_t hash = 0;
					in.getHash(hash);
					if(!read_packet(in, hash, uplink.get())) {
						log(ERROR).out << "Invalid input data: hash=" << vnl::hex(hash) << vnl::endl;
						usleep(error_interval*1000);
						break;
					}
				} else {
					log(ERROR).out << "Invalid input data: id=" << vnl::dec(id) << " size=" << vnl::dec(size) << vnl::endl;
					usleep(error_interval*1000);
					break;
				}
				int64_t now = vnl::currentTime();
				if(now - last_maintain > maintain_interval) {
					maintain();
					last_maintain = now;
				}
				poll(0);
			}
			::close(fd);
		}
		for(vnl::Address addr : table.keys()) {
			vnl::Router::close_t msg(std::make_pair(uplink.get(), addr));
			send(&msg, vnl::Router::instance);
		}
		for(vnl::Address addr : fwd_table.keys()) {
			vnl::Router::close_t msg(std::make_pair(uplink.get(), addr));
			send(&msg, vnl::Router::instance);
		}
	}
	
	bool read_packet(vnl::io::TypeInput& in, uint32_t hash, Uplink* uplink) {
		switch(hash) {
		case Uplink::open_t::MID: {
			vnl::Address addr;
			addr.deserialize(in, 0);
			in.skip(VNL_IO_INTERFACE, 0);
			vnl::Router::open_t msg(std::make_pair(uplink, addr));
			send(&msg, vnl::Router::instance);
			table[addr] = 1;
			break;
		}
		case Uplink::close_t::MID: {
			vnl::Address addr;
			addr.deserialize(in, 0);
			in.skip(VNL_IO_INTERFACE, 0);
			vnl::Router::close_t msg(std::make_pair(uplink, addr));
			send(&msg, vnl::Router::instance);
			table.erase(addr);
			break;
		}
		case vnl::Sample::PID: {
			vnl::Sample* sample = buffer.create<vnl::Sample>();
			sample->deserialize(in, 0);
			send_async(sample, sample->dst_addr);
			break;
		}
		case vnl::Frame::PID: {
			vnl::Frame* frame = buffer.create<vnl::Frame>();
			frame->deserialize(in, 0);
			int64_t& time = fwd_table[frame->src_addr];
			if(time == 0) {
				vnl::Router::open_t msg(std::make_pair(uplink, frame->src_addr));
				send(&msg, vnl::Router::instance);
			}
			time = vnl::currentTime();
			send_async(frame, frame->dst_addr);
			break;
		}
		default:
			return false;
		}
		return true;
	}
	
	void maintain() {
		// TODO
	}
	
	virtual vnl::io::Socket* connect() = 0;
	
protected:
	volatile bool dorun = true;
	vnl::String port;
	
private:
	vnl::io::Socket* sock;
	vnl::Map<vnl::Address, int> table;
	vnl::Map<vnl::Address, int64_t> fwd_table;
	
	
};



}

#endif /* CPP_INCLUDE_VNI_DOWNLINK_H_ */
