/*
 * TcpProxy.h
 *
 *  Created on: Dec 6, 2016
 *      Author: mwittal
 */

#ifndef INCLUDE_VNL_TCPPROXY_H_
#define INCLUDE_VNL_TCPPROXY_H_

#include <vnl/TcpProxySupport.hxx>


namespace vnl {

class TcpProxy : public TcpProxyBase {
public:
	TcpProxy(const vnl::String& domain_, const vnl::String& topic_)
		:	TcpProxyBase(domain_, topic_),
			running(false), sock(-1)
	{
	}
	
	int sock;
	
protected:
	void main() {
		for(const Topic& topic : export_topics) {
			subscribe(topic);
		}
		Super::main();
		log(INFO).out << "Client disconnected" << vnl::endl;
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
