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

class Client : public ClientBase {
public:
	Client()
		:	_error(0), _in(&_buf), _out(&_buf),
		 	req_num(0), timeout(1000000),
		 	do_fail(false)
	{
		src_addr = Address(local_domain, stream.get_mac());
		_data = Page::alloc();
	}
	
	~Client() {
		_data->free_all();
	}
	
	Client& operator=(const Address& addr) {
		set_address(addr);
		return *this;
	}
	
	void set_address(Hash64 domain, Hash64 topic) {
		dst_addr = vnl::Address(domain, topic);
	}
	
	void set_address(Topic topic) {
		dst_addr = vnl::Address(topic.domain, topic.name);
	}
	
	void set_address(Address addr) {
		dst_addr = addr;
	}
	
	Address get_address() const {
		return dst_addr;
	}
	
	void connect(vnl::Engine* engine) {
		stream.connect(engine);
		stream.subscribe(src_addr);
	}
	
	void set_timeout(int64_t timeout_ms) {
		timeout = timeout_ms*1000;
	}
	
	void set_fail(bool fail) {
		do_fail = fail;
	}
	
	virtual void serialize(vnl::io::TypeOutput& out) const {
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_BEGIN);
		out.putHash(VNI_HASH);
		dst_addr.serialize(out);
		out.putEntry(VNL_IO_INTERFACE, VNL_IO_END);
	}
	
	virtual void deserialize(vnl::io::TypeInput& in, int size) {
		dst_addr.deserialize(in, size);
	}
	
protected:
	Page* _data;
	vnl::io::ByteBuffer _buf;
	vnl::io::TypeInput _in;
	vnl::io::TypeOutput _out;
	int _error;
	
	Packet* _call() {
		_error = VNL_ERROR;
		_out.flush();
		req_num++;
		Frame* ret = 0;
		while(true) {
			if(Layer::have_shutdown) {
				_error = VNL_IO_EOF;
				break;
			}
			Frame frame;
			frame.src_addr = src_addr;
			frame.req_num = req_num;
			frame.data = _data;
			frame.size = _buf.position();
			stream.send(&frame, dst_addr);
			frame.data = 0;
			if(frame.count == 0 && do_fail) {
				_error = VNL_IO_EOF;
				break;
			}
			while(!Layer::have_shutdown) {
				Message* msg = stream.poll(timeout);
				if(msg) {
					if(msg->msg_id == vnl::Packet::MID) {
						if(((Packet*)msg)->pkt_id == vnl::Frame::PID) {
							Frame* pkt = (Frame*)((Packet*)msg)->payload;
							if(pkt->req_num == req_num) {
								ret = pkt;
								break;
							}
						}
					}
					msg->ack();
				}
			}
			if(ret || do_fail) {
				break;
			}
		}
		if(ret) {
			_error = VNL_SUCCESS;
			_buf.wrap(ret->data, ret->size);
			_in.reset();
		}
		return ret;
	}
	
private:
	Stream stream;
	Address src_addr;
	Address dst_addr;
	int64_t req_num;
	int64_t timeout;
	bool do_fail;
	
};



}

#endif /* INCLUDE_VNI_CLIENT_H_ */
