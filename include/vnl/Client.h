/*
 * Client.h
 *
 *  Created on: Jun 30, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNI_CLIENT_H_
#define INCLUDE_VNI_CLIENT_H_

#include <vnl/ClientSupport.hxx>
#include <vnl/Topic.hxx>
#include <vnl/Frame.h>
#include <vnl/Stream.h>
#include <vnl/Router.h>
#include <vnl/Layer.h>


namespace vnl {

enum {
	VNI_SUCCESS = 0, VNI_ERROR = -1
};

class Client : public ClientBase, public vnl::Stream {
public:
	Client()
		:	_error(0), _in(&_buf), _out(&_buf)
	{
		src = Address(local_domain, mac);
		_data = Page::alloc();
	}
	
	~Client() {
		if(is_connected) {
			Stream::close(src);
		}
		_data->free_all();
	}
	
	Client& operator=(const Address& addr) {
		set_address(addr);
		return *this;
	}
	
	virtual void set_address(const vnl::String& domain, const vnl::String& topic) {
		set_address(Hash64(domain), Hash64(topic));
	}
	
	void set_address(Hash64 domain, Hash64 topic) {
		dst = vnl::Address(domain, topic);
	}
	
	void set_address(Topic topic) {
		dst = vnl::Address(topic.domain, topic.name);
	}
	
	void set_address(Address addr) {
		dst = addr;
	}
	
	Address get_address() const {
		return dst;
	}
	
	void connect(vnl::Engine* engine) {
		if(is_connected) {
			Stream::close(dst);
		}
		Stream::connect(engine);
		Stream::open(src);
		is_connected = true;
	}
	
	void set_timeout(int64_t timeout_ms) {
		timeout = timeout_ms;
	}
	
	void set_fail(bool fail_if_timeout) {
		do_fail_if_timeout = fail_if_timeout;
	}
	
	virtual void serialize(vnl::io::TypeOutput& out) const {
		Writer wr(out);
		dst.serialize(out);
	}
	
	virtual void deserialize(vnl::io::TypeInput& in, int size) {
		dst.deserialize(in, size);
	}
	
protected:
	Page* _data;
	vnl::io::ByteBuffer _buf;
	vnl::io::TypeInput _in;
	vnl::io::TypeOutput _out;
	int _error;
	
	Packet* _call() {
		_out.flush();
		next_seq++;
		int64_t ts_begin = vnl::currentTimeMillis();
		Frame* ret = 0;
		while(true) {
			Frame frame;
			frame.src_addr = src;
			frame.data = _data;
			frame.size = _buf.position();
			frame.seq_num = next_seq;
			send(&frame, dst);
			frame.data = 0;
			Message* msg = poll(timeout*1000);
			if(msg) {
				if(msg->msg_id == vnl::Packet::MID) {
					if(((Packet*)msg)->pkt_id == vnl::Frame::PID) {
						ret = (Frame*)((Packet*)msg)->payload;
						break;
					}
				}
				msg->ack();
			}
			if(do_fail_if_timeout) {
				int64_t ts_now = vnl::currentTimeMillis();
				if(ts_now - ts_begin >= timeout) {
					break;
				}
			}
		}
		if(ret) {
			_buf.wrap(ret->data, ret->size);
			_in.reset();
			_error = VNI_SUCCESS;
		} else {
			_error = VNI_ERROR;
		}
		return ret;
	}
	
private:
	Address src;
	Address dst;
	uint32_t next_seq = 0;
	int64_t timeout = 1000;
	bool do_fail_if_timeout = false;
	bool is_connected = false;
	
};



}

#endif /* INCLUDE_VNI_CLIENT_H_ */
