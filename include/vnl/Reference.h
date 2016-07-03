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


/*
 * References should be on the Module::main() stack to allow for circular dependencies.
 */
template<typename T>
class Reference {
public:
	Reference(Engine* engine, T* obj);
	
	Reference(Engine* engine, uint64_t mac);
	
	Reference(Engine* engine, const char* name);
	
	Reference(Engine* engine, const vnl::String& name);
	
	~Reference() {
		close();
	}
	
	Reference(const Reference& other) = delete;
	Reference& operator=(const Reference& other) = delete;
	
	// will wait for module to be available
	T* get();
	
	// returns null if module is not available
	T* try_get();
	
	// manual release
	void close();
	
private:
	Engine* engine = 0;
	uint64_t mac = 0;
	T* obj = 0;
	
};


}

#endif /* INCLUDE_PHY_REFERENCE_H_ */
