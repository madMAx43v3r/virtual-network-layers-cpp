/*
 * PollServer.h
 *
 *  Created on: Jan 2, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_POLLSERVER_H_
#define INCLUDE_IO_POLLSERVER_H_

#include <unistd.h>
#include <fcntl.h>
#include <io/socket/Server.h>
#include <vector>
#include "util/simple_stack.h"
#include "util/simple_hashmap.h"

namespace vnl { namespace io { namespace poll {

using namespace vnl::phy;

class Server : public vnl::io::socket::Server {
public:
	Server(Engine* engine, const int N = 4) : vnl::io::socket::Server(engine), N(N) {
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
	
	~Server() {
		::close(pipefd[0]);
		::close(pipefd[1]);
	}
	
protected:
	struct pollfd_t : pollfd {
		pollfd_t() {
			fd = -1;
			events = 0;
			revents = 0;
		}
		int err() const {
			return revents & (POLLERR | POLLHUP | POLLNVAL);
		}
	};
	
	void handle(Message* msg) override {
		switch(msg->type) {
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
	
	void notify() override {
		char sig = 1;
		::write(pipefd[1], &sig, 1) == 1;
	}
	
	void wait(int millis) override {
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
	
	int update(key_t& key) {
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
	
	void expand() {
		N *= 2;
		fds.resize(N);
		keys.resize(N);
		for(int i = N/2; i < N; ++i) {
			empty.push(i);
		}
	}
	
private:
	int N;
	int pipefd[2];
	key_t pipekey;
	std::vector<pollfd_t> fds;
	std::vector<key_t> keys;
	vnl::util::simple_stack<int> empty;
	
};

}}}

#endif /* INCLUDE_IO_POLLSERVER_H_ */
