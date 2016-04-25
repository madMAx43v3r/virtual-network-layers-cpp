/*
 * Reference.h
 *
 *  Created on: Apr 24, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_REFERENCE_H_
#define INCLUDE_PHY_REFERENCE_H_

#include "phy/Registry.h"
#include "phy/Engine.h"


namespace vnl { namespace phy {

class Object;

class Reference {
public:
	Reference(Object* obj)
		:	mac(0), obj(obj)
	{
	}
	
	Reference(uint64_t mac)
		:	mac(mac)
	{
		assert(Engine::local);
	}
	
	Reference(const std::string& name) 
		:	Reference(Util::hash64(name))
	{
	}
	
	Reference(Object* parent, const std::string& name)
		:	Reference(parent->name + name)
	{
	}
	
	Reference(const Reference&) = delete;
	Reference& operator=(const Reference&) = delete;
	
	Object* get() {
		if(!obj) {
			obj = Engine::local->request<Object*>(Registry::connect_t(mac), Registry::instance);
		}
		return obj;
	}
	
	~Reference() {
		if(obj) {
			Engine::local->send(Registry::close_t(obj), Registry::instance);
		}
	}
	
private:
	uint64_t mac;
	Object* obj = 0;
	
	friend class Object;
	
};


}}

#endif /* INCLUDE_PHY_REFERENCE_H_ */
