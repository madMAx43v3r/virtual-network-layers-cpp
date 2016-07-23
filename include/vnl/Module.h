/*
 * Module.h
 *
 *  Created on: Jun 30, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_MODULE_H_
#define INCLUDE_VNL_MODULE_H_

#include <vnl/Object.h>
#include <vnl/Sample.h>
#include <vnl/Instance.hxx>
#include <vnl/ModuleSupport.hxx>


namespace vnl {

class Module : public ModuleBase {
public:
	Module(const vnl::String& domain, const vnl::String& topic)
		:	ModuleBase(domain, topic)
	{
	}
	
protected:
	virtual bool handle(Packet* pkt) {
		if(Super::handle(pkt)) {
			return true;
		}
		if(pkt->pkt_id == vnl::Sample::PID) {
			Sample* sample = (Sample*)pkt->payload;
			if(sample->data) {
				handle_switch(sample->data, pkt);
			}
		}
		return false;
	}
	
	virtual bool handle_switch(vnl::Value* sample, vnl::Packet* packet) {
		return false;
	}
	
};



}

#endif /* INCLUDE_VNL_MODULE_H_ */
