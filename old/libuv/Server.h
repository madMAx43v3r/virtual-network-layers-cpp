/*
 * server.h
 *
 *  Created on: Dec 24, 2015
 *      Author: mad
 */

#ifndef INCLUDE_IO_SERVER_H_
#define INCLUDE_IO_SERVER_H_

#include "Link.h"
using namespace vnl::phy;

#include <uv.h>

namespace vnl { namespace io {

class Server : public vnl::phy::Link {
public:
	static Server* instance;
	typedef std::function<void(uv_loop_t*, Message*)> callback_t;
	
	static void create(Engine* engine) {
		instance = new Server(engine);
		instance->start();
	}
	
	static void destroy() {
		delete instance;
	}
	
private:
	Server(Engine* engine) : Link(engine) {
		loop = uv_loop_new();
		uv_async_init(loop, &notify_handle, notify_async_cb);
		uv_async_init(loop, &exit_handle, exit_async_cb);
		notify_handle.data = this;
	}
	
	virtual ~Server() {
		for(auto buf : buffers) {
			free(buf.base);
		}
	}
	
public:
	void receive(Message* msg, Object* src) override {
		Link::receive(msg, src);
		if(src != this) {
			uv_async_send(&notify_handle);
		}
	}
	
public:
	typedef Generic<callback_t, 0x5dedc1fb> invoke_t;
	typedef Generic<uv_buf_t, 0x73be6d70> push_buffer_t;
	typedef Request<int, std::tuple<uv_write_t, uv_stream_t*, uv_buf_t>, 0x4fbe69e3> write_t;
	
	template<typename T>
	class invoke_req_t : public invoke_t {
	public:
		invoke_req_t(Server* server, const callback_t& func) : invoke_t(server, func) {}
		typedef T res_t;
		T res;
	};
	
	push_buffer_t push_buffer(const uv_buf_t& buf) {
		return push_buffer_t(this, buf);
	}
	
	write_t write(uv_stream_t* stream, const uv_buf_t& buf) {
		uv_write_t req;
		return write_t(this, std::make_tuple(req, stream, buf));
	}
	
	static uv_buf_t on_alloc(uv_handle_t* handle, size_t size) {
		uv_buf_t buf;
		if(!buffers.pop(buf)) {
			size = 4096;
			buf = uv_buf_init((char*)malloc(size), size);
			buffers.push(buf);
		}
		return buf;
	}
	
protected:
	void shutdown() override {
		uv_async_send(&exit_handle);
	}
	
	void handle(Message* msg) override {
		switch(msg->type) {
		case invoke_t::id:
			msg->cast<invoke_t>()->data(loop, msg);
			return;
		case push_buffer_t::id:
			buffers.push(msg->cast<push_buffer_t>()->data);
			break;
		case write_t::id: {
			auto& args = msg->cast<write_t>()->args;
			std::get<0>(args).data = msg;
			uv_write(&std::get<0>(args), std::get<1>(args), &std::get<2>(args), 1, on_write);
			return;
		}
		}
		msg->ack();
	}
	
private:
	void process() {
		while(dorun) {
			Message* msg = poll(0);
			if(msg) {
				receive(msg, this);
			} else {
				break;
			}
		}
	}
	
	void run() override {
		this_id = std::this_thread::get_id();
		uv_run(loop, UV_RUN_DEFAULT);
	}
	
	static void on_write(uv_write_t* req, int status) {
		write_t* msg = (write_t*)req->data;
		if(status < 0) {
			uv_close((uv_handle_t *)std::get<1>(msg->args), NULL);
		}
		msg->ack(status);
	}
	
	static void notify_async_cb(uv_async_t* handle, int status) {
		((Server*)handle->data)->process();
	}
	
	static void exit_async_cb(uv_async_t* handle, int status) {
		uv_close((uv_handle_t*)handle, NULL);
	}
	
private:
	static uv_loop_t* loop;
	static uv_async_t notify_handle;
	static uv_async_t exit_handle;
	
	static vnl::util::simple_queue<uv_buf_t> buffers;
	
};

Server* Server::instance = 0;

uv_loop_t* Server::loop = 0;
uv_async_t Server::notify_handle;
uv_async_t Server::exit_handle;

vnl::util::simple_queue<uv_buf_t> Server::buffers;

}}

#endif /* INCLUDE_IO_SERVER_H_ */
