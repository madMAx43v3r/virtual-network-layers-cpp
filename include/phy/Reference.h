/*
 * Reference.h
 *
 *  Created on: Apr 24, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_REFERENCE_H_
#define INCLUDE_PHY_REFERENCE_H_

#include "phy/Engine.h"


namespace vnl { namespace phy {

class Object;

class Reference {
public:
	Reference(Engine* engine, Object* obj);
	
	Reference(Engine* engine, uint64_t mac);
	
	Reference(Engine* engine, const std::string& name);
	
	Reference(Engine* engine, Object* parent, const std::string& name);
	
	~Reference() {
		close();
	}
	
	Reference(const Reference&) = delete;
	Reference& operator=(const Reference&) = delete;
	
	Object* get();
	
	void close();
	
private:
	uint64_t mac;
	Engine* engine = 0;
	Object* obj = 0;
	
};


}}

#endif /* INCLUDE_PHY_REFERENCE_H_ */
