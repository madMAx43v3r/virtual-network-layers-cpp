/*
 * TcpProxy.h
 *
 *  Created on: Dec 6, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_VNL_TCPPROXY_H_
#define INCLUDE_VNL_TCPPROXY_H_

#include <vnl/Pipe.h>
#include <vnl/TcpProxySupport.hxx>


namespace vnl {

class TcpProxy : public TcpProxyBase {
public:
	TcpProxy(const vnl::String& domain_, const vnl::String& topic_)
		:	TcpProxyBase(domain_, topic_),
			running(false), sock(-1), server(0)
	{
	}
	
	int sock;
	Pipe* server;
	
	typedef MessageType<Topic, 0x6e545b73> publish_t;
	typedef MessageType<Topic, 0x36e56b15> subscribe_t;
	typedef MessageType<uint64_t, 0xdaef2275> del_client_t;
	
protected:
	void main() {
		server->attach();
		Super::main();
		del_client_t msg(get_mac());
		send(&msg, server);
		server->detach();
	}
	
	bool handle(Message* msg) {
		if(msg->msg_id == publish_t::MID) {
			const Topic& topic = ((publish_t*)msg)->data;
			Object::subscribe(topic.domain, topic.name);
		} else if(msg->msg_id == subscribe_t::MID) {
			subscribe(((subscribe_t*)msg)->data.domain, ((subscribe_t*)msg)->data.name);
		}
		return Super::handle(msg);
	}
	
	int connect() {
		if(!running) {
			running = true;
			return sock;
		} else {
			return -1;
		}
	}
	
private:
	bool running;
	
};


} // vnl

#endif /* INCLUDE_VNL_TCPPROXY_H_ */
