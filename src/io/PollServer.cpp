/*
 * poll.Server.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include <unistd.h>
#include <fcntl.h>

#include "io/socket/PollServer.h"

using namespace vnl::phy;

namespace vnl { namespace io { namespace poll {

PollServer::PollServer() : N(4) {
	fds.resize(N);
	keys.resize(N);
	for(int i = 0; i < N; ++i) {
		empty.push(i);
	}
	if(::pipe2(pipefd, O_NONBLOCK) == 0) {
		pipekey.fd = pipefd[0];
		pipekey.events = POLLIN;
		update(pipekey);
	}
}

PollServer::~PollServer() {
	::close(pipefd[0]);
	::close(pipefd[1]);
}

bool PollServer::handle(Message* msg) {
	switch(msg->mid) {
	case poll_t::id: {
		poll_t* req = ((poll_t*)msg);
		req->res = update(req->args);
		msg->ack();
		return true;
	}
	case signal_t::id: {
		char buf[1024];
		while(read(pipefd[0], buf, sizeof(buf)) == sizeof(buf));
		pipekey.events = POLLIN;
		update(pipekey);
		msg->ack();
		return true;
	}
	}
	return false;
}

void PollServer::notify() {
	char sig = 1;
	::write(pipefd[1], &sig, 1) == 1;
}

void PollServer::wait(int millis) {
	unlock();
	int count = ::poll(&fds[0], N, millis);
	for(int i = 0; i < N && count > 0; ++i) {
		pollfd_t& pfd = fds[i];
		if(pfd.revents) {
			const key_t& key = keys[i];
			if(key.fd >= 0) {
				int err = pfd.err();
				if(pfd.revents & POLLIN) {
					key.sin->receive(new signal_t(err, true));
				}
				if(pfd.revents & POLLOUT) {
					key.sout->receive(new signal_t(err, true));
				}
			}
			pfd.events = 0;
			count--;
		}
	}
	lock();
}

int PollServer::update(key_t& key) {
	int& i = key.index;
	if(i < 0) {
		if(empty.empty()) {
			expand();
		}
		i = empty.top(); empty.pop();
	}
	if(key.fd >= 0) {
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

void PollServer::expand() {
	N *= 2;
	fds.resize(N);
	keys.resize(N);
	for(int i = N/2; i < N; ++i) {
		empty.push(i);
	}
}


}}}
