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
	Reference(Engine* engine, Object* obj)
		:	mac(obj->getMAC()), engine(engine), obj(obj)
	{
		engine->send_async(Registry::open_t(obj), Registry::instance);
	}
	
	Reference(Engine* engine, uint64_t mac)
		:	mac(mac), engine(engine)
	{
	}
	
	Reference(Engine* engine, const std::string& name) 
		:	Reference(engine, Util::hash64(name))
	{
	}
	
	Reference(Engine* engine, Object* parent, const std::string& name)
		:	Reference(engine, parent->name + name)
	{
	}
	
	~Reference() {
		close();
	}
	
	Reference(const Reference&) = delete;
	Reference& operator=(const Reference&) = delete;
	
	Object* get() {
		if(!obj) {
			obj = engine->request<Object*>(Registry::connect_t(mac), Registry::instance);
		}
		return obj;
	}
	
	void close() {
		if(obj) {
			engine->send_async(Registry::close_t(obj), Registry::instance);
			obj = 0;
		}
	}
	
private:
	uint64_t mac;
	Engine* engine = 0;
	Object* obj = 0;
	
};


}}

#endif /* INCLUDE_PHY_REFERENCE_H_ */
