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

class Client : public ClientBase, public vnl::Stream {
public:
	Client()
		:	_error(0), _in(&_buf), _out(&_buf),
		 	next_seq(1), timeout(1000),
		 	do_fail_if_timeout(false)
	{
		src = Address(local_domain, mac);
		_data = Page::alloc();
	}
	
	~Client() {
		if(Stream::get_engine()) {
			Stream::unsubscribe(src);
		}
		_data->free_all();
	}
	
	Client& operator=(const Address& addr) {
		set_address(addr);
		return *this;
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
		Stream::connect(engine);
		Stream::subscribe(src);
	}
	
	void set_timeout(int64_t timeout_ms) {
		timeout = timeout_ms;
	}
	
	void set_fail(bool fail_if_timeout) {
		do_fail_if_timeout = fail_if_timeout;
	}
	
	virtual void serialize(vnl::io::TypeOutput& out) const {
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_BEGIN);
		out.putHash(VNI_HASH);
		dst.serialize(out);
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_END);
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
		assert(Stream::get_engine());
		_out.flush();
		if(Layer::shutdown) {
			_error = VNL_IO_EOF;
			return 0;
		}
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
			_error = VNL_SUCCESS;
		} else {
			_error = VNL_ERROR;
		}
		return ret;
	}
	
private:
	Address src;
	Address dst;
	uint32_t next_seq;
	int64_t timeout;
	bool do_fail_if_timeout;
	
};



}

#endif /* INCLUDE_VNI_CLIENT_H_ */
