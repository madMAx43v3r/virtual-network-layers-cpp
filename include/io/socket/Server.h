/*
 * Server.h
 *
 *  Created on: Jan 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_SOCKET_SERVER_H_
#define INCLUDE_IO_SOCKET_SERVER_H_

#include "phy/FiberEngine.h"
#include "phy/Message.h"

namespace vnl { namespace io { namespace socket {

class Server : public vnl::phy::FiberEngine, public vnl::phy::Object {
public:
	static Server* instance;
	
	struct key_t {
		key_t() : obj(0), sin(0), sout(0), fd(-1), events(0), index(-1) {}
		vnl::phy::Object* obj;
		uint64_t sin;
		uint64_t sout;
		int fd;
		int events;
		int index;
	};
	
	typedef vnl::phy::Request<int, key_t, 0xd25c0daf> poll_t;
	typedef vnl::phy::Generic<int, 0x98a4ad8b> signal_t;
	
	int _prio = 0;
	
};

}}}

#endif /* INCLUDE_IO_SOCKET_SERVER_H_ */
