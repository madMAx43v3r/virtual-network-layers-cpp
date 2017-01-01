/*
 * Client.h
 *
 *  Created on: Jun 30, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNI_CLIENT_H_
#define INCLUDE_VNI_CLIENT_H_

#include <vnl/Frame.h>
#include <vnl/Stream.h>
#include <vnl/Router.h>
#include <vnl/Layer.h>

#include <vnl/TimeoutException.hxx>
#include <vnl/Topic.hxx>


namespace vnl {

class Client : public vnl::io::Serializable {
public:
	Client()
		:	_in(&_buf), _out(&_buf), _exception(0),
		 	req_num(0), timeout(1000000), do_fail(false)
	{
		src_addr = Address(local_domain_name, stream.get_mac());
		_data = Page::alloc();
	}
	
	virtual ~Client() {
		_data->free_all();
		vnl::destroy(_exception);
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
	
	void connect(vnl::Engine* engine, Router* target = 0) {
		stream.connect(engine, target ? target : Router::instance);
		stream.subscribe(src_addr);
	}
	
	void set_timeout(int64_t timeout_ms) {
		timeout = timeout_ms*1000;
	}
	
	void set_fail(bool fail) {
		do_fail = fail;
	}
	
	virtual void serialize(vnl::io::TypeOutput& out) const {
		dst_addr.serialize(out);
	}
	
	virtual void deserialize(vnl::io::TypeInput& in, int size) {
		dst_addr.deserialize(in, size);
	}
	
protected:
	Page* _data;
	vnl::io::ByteBuffer _buf;
	vnl::io::TypeInput _in;
	vnl::io::TypeOutput _out;
	vnl::Exception* _exception;
	
	Packet* _call(int type) {
		_out.flush();
		req_num++;
		Frame* ret = 0;
		while(true) {
			if(Layer::have_shutdown) {
				throw IOException();
			}
			Frame frame;
			frame.src_addr = src_addr;
			frame.type = type;
			frame.req_num = req_num;
			frame.data = _data;
			frame.size = _buf.position();
			stream.send(&frame, dst_addr);
			frame.data = 0;
			if(frame.count == 0 && do_fail) {
				throw IOException();
			}
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
			} else if(do_fail) {
				throw TimeoutException();
			}
			if(ret || do_fail) {
				break;
			}
		}
		if(ret) {
			_buf.wrap(ret->data, ret->size);
			_in.reset();
			if(ret->type == Frame::EXCEPTION) {
				ret->ack();
				vnl::Value* value = vnl::read(_in);
				if(value) {
					vnl::destroy(_exception);
					_exception = dynamic_cast<vnl::Exception*>(value);
					if(_exception) {
						_exception->raise();
					}
					vnl::destroy(value);
				}
				throw IOException();
			} else if(ret->type != Frame::RESULT) {
				ret->ack();
				throw IOException();
			}
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



} // vnl

#endif /* INCLUDE_VNI_CLIENT_H_ */
