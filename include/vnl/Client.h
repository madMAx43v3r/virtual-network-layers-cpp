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

class Client {
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
		stream.close();
	}
	
	Client& operator=(const Address& addr) {
		set_address(addr);
		return *this;
	}
	
	uint64_t get_mac() const {
		return stream.get_mac();
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
		stream.connect(engine, target);
		stream.subscribe(src_addr);
	}
	
	void set_timeout(int64_t timeout_ms) {
		timeout = timeout_ms*1000;
	}
	
	void set_fail(bool fail) {
		do_fail = fail;
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
		Packet* packet = 0;
		Frame* result = 0;
		while(true) {
			if(Layer::have_shutdown) {
				throw IOException();
			}
			
			Frame request;
			request.src_addr = src_addr;
			request.type = type;
			request.req_num = req_num;
			request.data = _data;
			request.size = _buf.position();
			
			stream.set_timeout(timeout/2);
			stream.send(&request, dst_addr);
			
			request.data = 0;
			if(do_fail) {
				if(request.count == 0) {
					throw IOException();
				}
				if(request.is_timeout) {
					throw TimeoutException();
				}
			}
			
			int left = request.count;
			Message* msg = stream.poll(timeout);
			while(msg) {
				left--;
				if(msg->msg_id == vnl::Packet::MID) {
					Packet* pkt = (Packet*)msg;
					if(pkt->pkt_id == vnl::Frame::PID) {
						Frame* frame = (Frame*)pkt->payload;
						if(frame->req_num == req_num) {
							packet = pkt;
							result = frame;
							if(result->type == Frame::RESULT || left == 0) {
								break;
							}
						}
					}
				}
				msg->ack();
				msg = stream.poll(timeout);
			}
			while(true) {
				msg = stream.poll(0);
				if(msg) {
					msg->ack();
				} else {
					break;
				}
			}
			if(result) {
				break;
			}
			if(do_fail) {
				throw TimeoutException();
			}
		}
		_buf.wrap(result->data, result->size);
		_in.reset();
		if(result->type == Frame::EXCEPTION) {
			packet->ack();
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
		}
		if(result->type != Frame::RESULT) {
			packet->ack();
			throw IOException();
		}
		return packet;
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
