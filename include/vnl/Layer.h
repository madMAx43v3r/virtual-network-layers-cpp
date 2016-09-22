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

#include <atomic>


namespace vnl {

extern uint64_t local_domain;
extern const char* local_domain_name;


class Process;

class Layer : public Actor {
public:
	Layer(const char* domain, const char* config_dir = 0);
	
	~Layer();
	
	void shutdown();
	
	void close();
	
	static const String* get_config(const String& domain, const String& topic, const String& name);
	
	static volatile bool have_shutdown;
	static std::atomic<int> num_threads;
	
protected:
	static void parse_config(const char* config_dir);
	
private:
	static Map<String, String>* config;
	
	bool closed;
	
	friend class Process;
	
};



}

#endif /* INCLUDE_LAYER_H_ */
