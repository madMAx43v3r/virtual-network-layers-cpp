/*
 * PollServer.h
 *
 *  Created on: Jan 2, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_POLL_SERVER_H_
#define INCLUDE_IO_POLL_SERVER_H_

#include <sys/poll.h>
#include <vector>
#include <stack>

#include "io/socket/Server.h"
#include "phy/FiberEngine.h"

namespace vnl { namespace io { namespace poll {

class Server : public vnl::io::socket::Server {
public:
	Server();
	~Server();
	
	virtual void run() override {}
	
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
	
	virtual bool handle(vnl::phy::Message* msg) override;
	
	virtual void notify() override;
	virtual void wait(int millis) override;
	
	int update(key_t& key);
	void expand();
	
private:
	int N;
	int pipefd[2];
	key_t pipekey;
	std::vector<pollfd_t> fds;
	std::vector<key_t> keys;
	std::stack<int, std::vector<int> > empty;
	
	struct init;
	static init initializer;
	
};


}}}

#endif /* INCLUDE_IO_POLL_SERVER_H_ */
