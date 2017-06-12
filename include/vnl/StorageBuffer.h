/*
 * StorageBuffer.h
 *
 *  Created on: Apr 4, 2017
 *      Author: mad
 */

#ifndef INCLUDE_VNL_STORAGEBUFFER_H_
#define INCLUDE_VNL_STORAGEBUFFER_H_

#include <vnl/StorageBufferSupport.hxx>


namespace vnl {

class StorageBuffer : public StorageBufferBase {
public:
	StorageBuffer(const vnl::String& domain_, const vnl::String& topic_)
		:	StorageBufferBase(domain_, topic_)
	{
	}
	
protected:
	void main() {
		for(Address& addr : topics) {
			subscribe(addr);
		}
		run();
	}
	
	void handle(const Entry& sample) {
		put_entry(&sample);
	}
	
	void put_entry(const Pointer<Entry>& value) {
		put_entry(value.get());
	}
	
	Pointer<Entry> get_entry(const Hash64& key) const {
		const Pointer<Entry>* p_entry = index.find(key);
		return p_entry ? p_entry->clone() : 0;
	}
	
	Array<Pointer<Entry> > get_entries(const Array<Hash64>& keys) const {
		Array<Pointer<Entry> > res;
		for(Hash64 key : keys) {
			res.push_back(get_entry(key));
		}
		return res;
	}
	
	void delete_entry(const Hash64& key) {
		index.erase(key);
	}
	
private:
	void put_entry(const Entry* value) {
		if(value) {
			Pointer<Entry>& entry = index[value->key];
			if(!entry || value->version > entry->version) {
				entry = value->clone();
			}
		}
	}
	
private:
	Map<Hash64, Pointer<Entry> > index;
	
};


} // vnl

#endif /* INCLUDE_VNL_STORAGEBUFFER_H_ */
