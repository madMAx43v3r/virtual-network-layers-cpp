/*
 * Reference.h
 *
 *  Created on: Apr 24, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_REFERENCE_H_
#define INCLUDE_PHY_REFERENCE_H_

#include "vnl/Engine.h"
#include "vnl/Registry.h"
#include "vnl/String.h"


namespace vnl {

class Module;


template<typename T>
class Reference {
public:
	Reference(Module* module, T* obj);
	
	Reference(Module* module, uint64_t mac);
	
	Reference(Module* module, const char* name);
	
	Reference(Module* module, const vnl::String& name);
	
	~Reference() {
		close();
	}
	
	Reference(const Reference& other) = delete;
	Reference& operator=(const Reference& other) = delete;
	
	T* get();
	
	void close();
	
private:
	uint64_t mac = 0;
	Module* module = 0;
	T* obj = 0;
	
};


}

#endif /* INCLUDE_PHY_REFERENCE_H_ */
