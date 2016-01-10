/*
 * Struct.h
 *
 *  Created on: Jan 10, 2016
 *      Author: mad
 */

#ifndef INCLUDE_STRUCT_H_
#define INCLUDE_STRUCT_H_

#include <unordered_map>
#include <functional>
#include <mutex>

#include "io/Stream.h"

namespace vnl {

class Struct {
public:
	virtual ~Struct() {}
	
	virtual bool serialize(vnl::io::Stream* stream) = 0;
	virtual bool deserialize(vnl::io::Stream* stream) = 0;
	
	uint64_t vnl_id = 0;
	
	struct type_t {
		uint64_t id = 0;
		std::function<Struct*()> create;
	};
	
	static void define(uint64_t id, const type_t& type) {
		std::unique_lock<std::mutex> lock(mutex);
		registry[id] = type;
	}
	
	static type_t resolve(uint64_t id) {
		std::unique_lock<std::mutex> lock(mutex);
		auto iter = registry.find(id);
		if(iter != registry.end()) {
			return iter->second;
		}
		return 0;
	}
	
private:
	static std::mutex mutex;
	static std::unordered_map<uint64_t, type_t> registry;
	
};


}

#endif /* INCLUDE_STRUCT_H_ */
