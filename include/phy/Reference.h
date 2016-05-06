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
		:	mac(obj->getMAC()), obj(obj)
	{
		assert(Engine::local);
		Engine::local->send_async(Registry::open_t(obj), Registry::instance);
	}
	
	Reference(uint64_t mac)
		:	mac(mac)
	{
	}
	
	Reference(const std::string& name) 
		:	Reference(Util::hash64(name))
	{
	}
	
	Reference(Object* parent, const std::string& name)
		:	Reference(parent->name + name)
	{
	}
	
	~Reference() {
		close();
	}
	
	Reference(const Reference&) = delete;
	Reference& operator=(const Reference&) = delete;
	
	Object* get() {
		if(!obj) {
			obj = Engine::local->request<Object*>(Registry::connect_t(mac), Registry::instance);
		}
		return obj;
	}
	
	void close() {
		if(obj) {
			Engine::local->send_async(Registry::close_t(obj), Registry::instance);
			obj = 0;
		}
	}
	
private:
	uint64_t mac;
	Object* obj = 0;
	
	friend class Object;
	
};


}}

#endif /* INCLUDE_PHY_REFERENCE_H_ */
