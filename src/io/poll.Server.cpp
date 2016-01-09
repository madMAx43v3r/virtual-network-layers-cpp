/*
 * poll.Server.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include <unistd.h>
#include <fcntl.h>

#include "io/poll/Server.h"

using namespace vnl::phy;

namespace vnl { namespace io { namespace poll {

Server::Server(Engine* engine, const int N) : vnl::io::socket::Server(engine), N(N) {
	fds.resize(N);
	keys.resize(N);
	for(int i = 0; i < N; ++i) {
		empty.push(i);
	}
	if(::pipe2(pipefd, O_NONBLOCK) == 0) {
		pipekey.obj = this;
		pipekey.fd = pipefd[0];
		pipekey.events = POLLIN;
		update(pipekey);
	}
}

Server::~Server() {
	::close(pipefd[0]);
	::close(pipefd[1]);
}

void Server::handle(Message* msg) {
	switch(msg->mid) {
	case poll_t::id: {
		poll_t* req = msg->cast<poll_t>();
		req->res = update(req->args);
		break;
	}
	case signal_t::id: {
		char buf[1024];
		while(read(pipefd[0], buf, sizeof(buf)) == sizeof(buf));
		pipekey.events = POLLIN;
		update(pipekey);
		break;
	}
	}
	msg->ack();
}

void Server::notify() {
	char sig = 1;
	::write(pipefd[1], &sig, 1) == 1;
}

void Server::wait(int millis) {
	unlock();
	int count = ::poll(&fds[0], N, millis);
	for(int i = 0; i < N && count > 0; ++i) {
		pollfd_t& pfd = fds[i];
		if(pfd.revents) {
			const key_t& key = keys[i];
			if(key.obj) {
				int err = pfd.err();
				if(pfd.revents & POLLIN) {
					signal_t* msg = new signal_t(err, true);
					msg->sid = key.sin;
					key.obj->receive(msg);
				}
				if(pfd.revents & POLLOUT) {
					signal_t* msg = new signal_t(err, true);
					msg->sid = key.sout;
					key.obj->receive(msg);
				}
			}
			pfd.events = 0;
			count--;
		}
	}
	lock();
}

int Server::update(key_t& key) {
	int& i = key.index;
	if(i < 0) {
		if(empty.empty()) {
			expand();
		}
		i = empty.top(); empty.pop();
	}
	if(key.obj) {
		fds[i].fd = key.fd;
		fds[i].events = key.events;
		keys[i] = key;
	} else {
		fds[i] = pollfd_t();
		keys[i] = key_t();
		empty.push(i);
		i = -1;
	}
	return i;
}

void Server::expand() {
	N *= 2;
	fds.resize(N);
	keys.resize(N);
	for(int i = N/2; i < N; ++i) {
		empty.push(i);
	}
}


}}}
