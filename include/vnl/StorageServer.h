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
		if(!filename.empty()) {
			open();
			if(file && !readonly) {
				set_timeout(interval, std::bind(&StorageServer::update, this), VNL_TIMER_REPEAT);
			}
		}
		for(Address& addr : topics) {
			subscribe(addr);
		}
		run();
		if(file) {
			::fclose(file);
		}
	}
	
	void handle(const Entry& sample) {
		if(!readonly) {
			put_entry(&sample);
		}
	}
	
	void put_entry(const Pointer<Entry>& value) {
		if(!readonly) {
			put_entry(value.get());
		}
	}
	
	Pointer<Entry> get_entry(const Hash64& key) const {
		Pointer<Entry> res;
		const Vector<int64_t,2>* p_ind = index.find(key);
		if(p_ind) {
			::fseek(file, (*p_ind)[1], SEEK_SET);
			((vnl::io::TypeInput&)in).reset();
			vnl::read((vnl::io::TypeInput&)in, res);
		}
		return res;
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
		if(!readonly) {
			Entry dummy;
			dummy.key = key;
			dummy.version = -1;
			write_entry(&dummy);
		}
	}
	
	void update() {
		::fflush(file);
	}
	
private:
	void put_entry(const Entry* value) {
		Vector<int64_t,2>& ind = index[value->key];
		if(value->version > ind[0]) {
			ind[0] = value->version;
			ind[1] = file_begin + out.get_output_pos();
			write_entry(value);
		}
	}
	
	void write_entry(const Entry* value) {
		::fseek(file, file_begin + out.get_output_pos(), SEEK_SET);
		vnl::write(out, value);
		out.flush();
	}
	
	void open() {
		char buf[1024];
		filename.to_string(buf, sizeof(buf));
		if(readonly) {
			file = ::fopen(buf, "rb");
		} else if(truncate) {
			file = ::fopen(buf, "wb+");
		} else {
			FILE* tmp = ::fopen(buf, "ab+");
			::fclose(tmp);
			file = ::fopen(buf, "rb+");
		}
		if(!file.good()) {
			log(ERROR).out << "Unable to open file: " << filename << vnl::endl;
			return;
		}
		if(!readonly) {
			readonly = true;
			while(::flock(file.get_fd(), LOCK_EX | LOCK_NB)) {
				log(ERROR).out << "Failed to lock file: " << filename << vnl::endl;
				sleep(10);
				if(!vnl_dorun) {
					return;
				}
			}
			readonly = false;
		}
		log(INFO).out << "Running on file: " << filename << vnl::endl;
		read_all();
	}
	
	void read_all() {
		in.reset();
		::fseek(file, 0, SEEK_SET);
		while(true) {
			file_begin = in.get_input_pos();
			Pointer<Entry> entry;
			vnl::read(in, entry);
			if(in.error()) {
				if(in.error() != VNL_IO_EOF) {
					log(ERROR).out << "Read error at " << file_begin << vnl::endl;
				}
				break;
			}
			if(entry) {
				if(entry->version > 0) {
					Vector<int64_t,2>& ind = index[entry->key];
					ind[0] = entry->version;
					ind[1] = file_begin;
				} else {
					index.erase(entry->key);
				}
			}
		}
		log(INFO).out << "Found " << index.size() << " entries, " << in.get_num_read()/1024 << " kB" << vnl::endl;
	}
	
private:
	vnl::io::File file;
	vnl::io::TypeInput in;
	vnl::io::TypeOutput out;
	
	Map<Hash64, Vector<int64_t,2> > index;
	int64_t file_begin = 0;
	
};


} // vnl

#endif /* INCLUDE_VNL_STORAGESERVER_H_ */
