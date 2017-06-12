/*
 * Layer.h
 *
 *  Created on: May 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_LAYER_H_
#define INCLUDE_VNL_LAYER_H_

#include <vnl/Map.h>
#include <vnl/InputPin.h>
#include <vnl/OutputPin.h>
#include <vnl/info/Type.hxx>

#include <atomic>


namespace vnl {

class Process;
class GlobalPool;


extern const char* local_domain_name;
extern const char* local_config_name;


namespace internal {
	
	extern Map<Hash32, vnl::info::Type>* type_info_;
	
} // internal


/*
 * Returns all type infos known to this process.
 */
Map<Hash32, vnl::info::Type> get_type_info();

/*
 * Returns type info for specified type, null if not found.
 */
const vnl::info::Type* get_type_info(Hash32 type_name);


/*
 * Input/Output pin functions
 */
inline void connect(InputPin& input, OutputPin& output) {
	input.attach(output.create());
}

template<typename A, typename B>
void connect(TypedInputPin<A>& input, TypedOutputPin<B>& output) {
	A* assign_test = (B*)0;
	input.attach(output.create());
}

template<typename A>
void connect(TypedInputPin<A>& input, OutputPin& output) {
	A* assign_test = (vnl::Value*)0;
	input.attach(output.create());
}


class Layer {
public:
	Layer(const char* domain, const char* config_dir = 0);
	
	~Layer();
	
	static void shutdown();
	
	static volatile bool have_shutdown;
	static std::atomic<int> num_threads;
	
protected:
	static void parse_config(const char* config_dir);
	
};


} // vnl

#endif /* INCLUDE_VNL_LAYER_H_ */
