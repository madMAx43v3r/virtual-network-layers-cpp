/*
 * Layer.h
 *
 *  Created on: May 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_LAYER_H_
#define INCLUDE_LAYER_H_

#include <vnl/Basic.h>
#include <vnl/Map.h>


namespace vnl {

extern uint64_t local_domain;
extern const char* local_domain_name;


class Process;

class Layer : public Actor {
public:
	Layer(const char* domain, const char* config_dir = 0);
	
	~Layer();
	
	static const String* get_config(String domain, String topic, String name);
	
	static volatile bool shutdown;
	
protected:
	static void parse_config(const char* config_dir);
	
private:
	static volatile bool finished;
	
	static Map<String, String> config;
	
	friend class Process;
	
};



}

#endif /* INCLUDE_LAYER_H_ */
