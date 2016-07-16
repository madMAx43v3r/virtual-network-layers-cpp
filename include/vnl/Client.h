/*
 * Client.h
 *
 *  Created on: Jun 30, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNI_CLIENT_H_
#define INCLUDE_VNI_CLIENT_H_

#include <vnl/ClientSupport.hxx>
#include <vnl/Frame.h>
#include <vnl/Stream.h>
#include <vnl/Router.h>


namespace vnl {

enum {
	VNI_SUCCESS = 0, VNI_ERROR = -1
};

class Client : public ClientBase, public vnl::Stream {
public:
	Client()
		:	_error(0), _in(&_buf), _out(&_buf)
	{
		data = vnl::Page::alloc();
		_buf.wrap(data);
	}
	
	~Client() {
		if(is_connected) {
			Stream::close(dst);
		}
		data->free_all();
	}
	
	virtual void set_address(const vnl::String& domain, const vnl::String& topic) {
		set_address(domain, topic);
	}
	
	void set_address(Hash64 domain, Hash64 topic) {
		dst = vnl::Address(domain, topic);
	}
	
	void connect(vnl::Engine* engine) {
		if(is_connected) {
			Stream::close(dst);
		}
		Stream::connect(engine);
		Stream::open(dst);
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
	vnl::io::ByteBuffer _buf;
	vnl::io::TypeInput _in;
	vnl::io::TypeOutput _out;
	int _error;
	
	vnl::Packet* _call() {
		_out.flush();
		next_seq++;
		int64_t ts_begin = vnl::currentTimeMillis();
		Frame* ret = 0;
		while(true) {
			Frame frame;
			frame.data = data;
			frame.size = _buf.position();
			frame.seq_num = next_seq;
			send(&frame, dst);
			frame.data = 0;
			Message* msg = poll(timeout*1000);
			if(msg) {
				if(msg->msg_id == vnl::Packet::MID) {
					if(((Packet*)msg)->pkt_id == vnl::Frame::PID) {
						ret = (Frame*)msg;
						break;
					}
				}
				msg->ack();
			}
			std::cout << "vnl::Client TIMEOUT" << std::endl;
			if(do_fail_if_timeout) {
				int64_t ts_now = vnl::currentTimeMillis();
				if(ts_now - ts_begin >= timeout) {
					break;
				}
			}
		}
		if(ret) {
			_buf.wrap(ret->data, ret->size);
			_error = VNI_SUCCESS;
		} else {
			_error = VNI_ERROR;
		}
		return ret;
	}
	
private:
	vnl::Address dst;
	vnl::Page* data;
	uint32_t next_seq = 0;
	int64_t timeout = 1000;
	bool do_fail_if_timeout = false;
	bool is_connected = false;
	
};



}

#endif /* INCLUDE_VNI_CLIENT_H_ */
