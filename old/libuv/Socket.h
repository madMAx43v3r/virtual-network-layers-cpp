/*
 * socket.h
 *
 *  Created on: Dec 24, 2015
 *      Author: mad
 */

#ifndef INCLUDE_IO_SOCKET_H_
#define INCLUDE_IO_SOCKET_H_

#include <string.h>

#include "Server.h"
#include "Link.h"
using namespace vnl::phy;
using namespace std::placeholders;

namespace vnl { namespace io {

class Socket : public vnl::phy::Object {
public:
	Socket(Object* parent) : Object(parent), server(Server::instance), loop(0),
		ctrl(this), in(this), out(this)
	{
		socket.data = this;
		resolver.data = this;
	}
	~Socket() {
		close();
	}
	
private:
	Socket(Object* parent, const uv_tcp_t& tcp) : Socket(parent)
	{
		socket = tcp;
		socket.data = this;
	}
	
	struct buffer_t {
		buffer_t() : current(0), remaining(0), status(0) {
			buf.base = 0;
			buf.len = 0;
		}
		uv_buf_t buf;
		char* current;
		int remaining;
		int status;
	};
	
	typedef Server::invoke_req_t<int> create_t;
	typedef Server::invoke_req_t<int> connect_t;
	typedef Server::invoke_req_t<int> bind_t;
	typedef Server::invoke_req_t<int> listen_t;
	typedef Generic<buffer_t, 0xd0606d39> read_t;
	typedef Generic<uv_tcp_t, 0x158d9722> accept_t;
	
public:
	int create() {
		return ctrl.request<int>(create_t(server, std::bind(&Socket::create_cb, this, _1, _2)));
	}
	
	int connect(std::string endpoint, int port) {
		return ctrl.request<int>(connect_t(server, std::bind(&Socket::connect_cb, this, _1, _2, endpoint, port)));
	}
	
	int bind(int port) {
		return ctrl.request<int>(bind_t(server, std::bind(&Socket::bind_cb, this, _1, _2, port)));
	}
	
	int listen() {
		return ctrl.request<int>(listen_t(server, std::bind(&Socket::listen_cb, this, _1, _2)));
	}
	
	Socket* accept() {
		uv_tcp_t tcp = ctrl.read<accept_t>().data;
		if(tcp.data) {
			Socket* sock = new Socket(this, tcp);
			sock->read_start();
			return sock;
		} else {
			return 0;
		}
	}
	
	int read(void* buf, int len) {
		int left = len;
		while(left > 0) {
			while(input.remaining <= 0) {
				if(input.buf.base) {
					send(server->push_buffer(input.buf));
				}
				if(input.status < 0) {
					return input.status;
				}
				input = in.read<read_t>().data;
			}
			int n = std::min(left, input.remaining);
			memcpy(buf, input.current, n);
			input.current += n;
			input.remaining -= n;
			left -= n;
		}
		return len;
	}
	
	int write(const void* buf, int len) {
		uv_buf_t data;
		data.base = (char*)buf;
		data.len = len;
		return out.request<int>(server->write((uv_stream_t*)&socket, data));
	}
	
	void close() {
		send(Server::invoke_t(server, std::bind(&Socket::close_cb, this, _1, _2)));
	}
	
private:
	void read_start() {
		send(Server::invoke_t(server, std::bind(&Socket::read_start_cb, this, _1, _2)));
	}
	
private:
	void create_cb(uv_loop_t* loop, Message* msg) {
		msg->cast<create_t>()->res = uv_tcp_init(loop, &socket);
		msg->ack();
	}
	void connect_cb(uv_loop_t* loop, Message* msg, std::string endpoint, int port) {
		connection.data = msg;
		struct addrinfo hints;
		hints.ai_family = PF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = 0;
		char port_[16];
		sprintf(port_, "%d", port);
		int err = uv_getaddrinfo(loop, &resolver, on_resolved, endpoint.c_str(), port_, &hints);
		if(err != 0) {
			msg->cast<connect_t>()->res = err;
			msg->ack();
		}
	}
	void bind_cb(uv_loop_t* loop, Message* msg, int port) {
		struct sockaddr_in addr = uv_ip4_addr("0.0.0.0", port);
		msg->cast<bind_t>()->res = uv_tcp_bind(&socket, addr);
		msg->ack();
	}
	void listen_cb(uv_loop_t* loop, Message* msg) {
		this->loop = loop;
		msg->cast<listen_t>()->res = uv_listen((uv_stream_t*)&socket, 100, on_new_connection);
		msg->ack();
	}
	void read_start_cb(uv_loop_t* loop, Message* msg) {
		uv_read_start((uv_stream_t*)&socket, Server::on_alloc, on_read);
		msg->ack();
	}
	void close_cb(uv_loop_t* loop, Message* msg) {
		uv_close((uv_handle_t*)&socket, NULL);
		msg->ack();
	}
	
	static void on_resolved(uv_getaddrinfo_t* resolver, int status, struct addrinfo* res) {
		Socket* sock = (Socket*)resolver->data;
		sockaddr_in* dst = (struct sockaddr_in*)res->ai_addr;
		int err = uv_tcp_connect(&sock->connection, &sock->socket, *dst, on_connect);
		if(err != 0) {
			connect_t* msg = (connect_t*)sock->connection.data;
			msg->res = err;
			msg->ack();
		}
	}
	
	static void on_connect(uv_connect_t* connection, int status) {
		auto* msg = (connect_t*)connection->data;
		if(status >= 0) {
			uv_read_start(connection->handle, Server::on_alloc, on_read);
		}
		msg->res = status;
		msg->ack();
	}
	
	static void on_read(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
		Socket* sock = (Socket*)stream->data;
		buffer_t buffer;
		if(nread < 0) {
			if(nread == UV_EOF) {
				uv_close((uv_handle_t *)&sock->socket, NULL);
			}
			buffer.status = nread;
		}
		buffer.buf = buf;
		buffer.current = buf.base;
		buffer.remaining = nread;
		sock->in.receive(new read_t(buffer));
	}
	
	static void on_new_connection(uv_stream_t* server, int status) {
		Socket* sock = (Socket*)server->data;
		uv_tcp_t client;
		client.data = 0;
		if(status >= 0) {
			uv_tcp_init(sock->loop, &client);
			if(uv_accept(server, (uv_stream_t*)&client) == 0) {
				client.data = sock;
			} else {
				uv_close((uv_handle_t*)&client, NULL);
				return;
			}
		}
		sock->ctrl.receive(new accept_t(client));
	}
	
private:
	Server* server;
	uv_loop_t* loop;
	Stream ctrl;
	Stream in;
	Stream out;
	
	uv_tcp_t socket;
	uv_getaddrinfo_t resolver;
	uv_connect_t connection;
	buffer_t input;
	
};

}}

#endif /* INCLUDE_IO_SOCKET_H_ */
