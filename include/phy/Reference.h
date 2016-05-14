/*
 * Reference.h
 *
 *  Created on: Apr 24, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_REFERENCE_H_
#define INCLUDE_PHY_REFERENCE_H_

#include "phy/Engine.h"
#include "phy/Registry.h"
#include "String.h"


namespace vnl { namespace phy {

class Object;


template<typename T>
class Reference {
public:
	Reference(Engine* engine, T* obj);
	
	Reference(Engine* engine, uint64_t mac);
	
	Reference(Engine* engine, const vnl::String& name);
	
	Reference(Engine* engine, Object* parent, const vnl::String& name);
	
	T* get() {
		if(!obj) {
			Registry::connect_t req(mac);
			engine->send(engine, &req, Registry::instance);
			obj = req.res;
		}
		return obj;
	}
	
	void close() {
		if(obj) {
			Registry::close_t msg(obj);
			engine->send(engine, &msg, Registry::instance);
			obj = 0;
		}
	}
	
private:
	uint64_t mac;
	Engine* engine = 0;
	T* obj = 0;
	
};


}}

#endif /* INCLUDE_PHY_REFERENCE_H_ */
