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
#include "util/spinlock.h"

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
		mutex.lock();
		registry[id] = type;
		mutex.unlock();
	}
	
	static type_t resolve(uint64_t id) {
		type_t ret;
		mutex.lock();
		auto iter = registry.find(id);
		if(iter != registry.end()) {
			ret = iter->second;
		}
		mutex.unlock();
		return ret;
	}
	
private:
	static util::spinlock mutex;
	static std::unordered_map<uint64_t, type_t> registry;
	
};


}

#endif /* INCLUDE_STRUCT_H_ */
