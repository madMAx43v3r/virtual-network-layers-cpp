/*
 * Server.h
 *
 *  Created on: Jan 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_POLL_SERVER_H_
#define INCLUDE_IO_POLL_SERVER_H_

#include <sys/poll.h>

#include "phy/Link.h"

namespace vnl { namespace io { namespace socket {

using namespace vnl::phy;

class Server : public vnl::phy::Link {
public:
	static Server* instance;
	
	Server(Engine* engine) : Link(engine) {}
	
	struct key_t {
		key_t() : obj(0), sin(0), sout(0), fd(-1), events(0), index(-1) {}
		Object* obj;
		uint64_t sin;
		uint64_t sout;
		int fd;
		int events;
		int index;
	};
	
	typedef Request<int, key_t, 0xd25c0daf> poll_t;
	typedef Generic<int, 0x98a4ad8b> signal_t;
	
	
};

Server* Server::instance = 0;

}}}

#endif /* INCLUDE_IO_POLL_SERVER_H_ */
