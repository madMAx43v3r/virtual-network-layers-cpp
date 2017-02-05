/*
 * StorageServer.h
 *
 *  Created on: Feb 4, 2017
 *      Author: mad
 */

#ifndef INCLUDE_VNL_STORAGESERVER_H_
#define INCLUDE_VNL_STORAGESERVER_H_

#include <vnl/Vector.h>
#include <vnl/StorageServerSupport.hxx>


namespace vnl {

class StorageServer : public StorageServerBase {
public:
	StorageServer(const vnl::String& domain_, const vnl::String& topic_)
		:	StorageServerBase(domain_, topic_),
			in(&file), out(&file)
	{
	}
	
protected:
	void main() {
		char buf[1024];
		String filepath = filename << ".dat";
		filepath.to_string(buf, sizeof(buf));
		file = ::fopen(buf, "a+");
		if(!file.good()) {
			log(ERROR).out << "Unable to open file: " << filepath << vnl::endl;
			return;
		}
		
		log(INFO).out << "Reading storage file ..." << vnl::endl;
		::fseek(file, 0, SEEK_SET);
		while(true) {
			int64_t file_pos = in.get_input_pos();
			Pointer<Entry> entry;
			vnl::read(in, entry);
			if(in.error()) {
				break;
			}
			if(entry) {
				if(entry->version > 0) {
					Vector<int64_t,2>& ind = index[entry->key];
					ind[0] = entry->version;
					ind[1] = file_pos;
				} else {
					index.erase(entry->key);
				}
			}
		}
		::fseek(file, 0, SEEK_END);
		log(INFO).out << "Found " << index.size() << " entries, " << in.get_num_read()/1024 << " kB" << vnl::endl;
		
		log(INFO).out << "Storing to file: " << filepath << vnl::endl;
		run();
		
		::fflush(file);
		::fclose(file);
	}
	
	void handle(const Entry& sample) {
		write_entry(&sample);
	}
	
	void put_entry(const Pointer<Entry>& value) {
		write_entry(value.value());
	}
	
	Pointer<Entry> get_entry(const Hash64& key) const {
		Pointer<Entry> res;
		Vector<int64_t,2>* p_ind = index.find(key);
		if(p_ind) {
			::fseek(file, (*p_ind)[1], SEEK_SET);
			((vnl::io::TypeInput&)in).reset();
			vnl::read((vnl::io::TypeInput&)in, res);
			::fseek(file, 0, SEEK_END);
		}
		return res;
	}
	
	void delete_entry(const Hash64& key) {
		index.erase(key);
		Entry dummy;
		dummy.key = key;
		dummy.version = -1;
		vnl::write(out, dummy);
		out.flush();
	}
	
private:
	void write_entry(const Entry* value) {
		Vector<int64_t,2>& ind = index[value->key];
		if(value->version > ind[0]) {
			ind[0] = value->version;
			ind[1] = out.get_output_pos();
			vnl::write(out, value);
			out.flush();
		}
	}
	
private:
	vnl::io::File file;
	vnl::io::TypeInput in;
	vnl::io::TypeOutput out;
	
	Map<Hash64, Vector<int64_t,2> > index;
	
};


} // vnl

#endif /* INCLUDE_VNL_STORAGESERVER_H_ */
