/*
 * Database.h
 *
 *  Created on: Nov 24, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_DATABASE_H_
#define INCLUDE_VNL_DATABASE_H_

#include <vnl/io.h>
#include <vnl/DatabaseSupport.hxx>


namespace vnl {

class Database : public DatabaseBase {
public:
	Database(const vnl::String& domain_, const vnl::String& topic_)
		:	DatabaseBase(domain_, topic_), out(&file)
	{
	}
	
protected:
	void main() {
		char buf[1024];
		filename.to_string(buf, sizeof(buf));
		file = ::fopen(buf, "a+");
		if(!file.good()) {
			log(ERROR).out << "Unable to open file: " << filename << vnl::endl;
			return;
		}
		log(INFO).out << "Reading database ..." << vnl::endl;
		::fseek(file, 0, SEEK_SET);
		read_all();
		::fseek(file, 0, SEEK_END);
		log(INFO).out << "Finished reading database." << vnl::endl;
		run();
		::fflush(file);
		::fclose(file);
	}
	
	bool handle(Frame* frame) {
		if(out.error()) {
			return false;
		}
		if(frame->type == Frame::CALL) {
			out.writeBinary(frame->data, frame->size);
			out.flush();
			if(out.error()) {
				log(ERROR).out << "Failed to write: " << out.error() << vnl::endl;
				log(INFO).out << "Database is now freezed!" << vnl::endl;
			}
		}
		return Super::handle(frame);
	}
	
private:
	void read_all() {
		vnl::io::TypeInput in(&file);
		while(!in.error()) {
			int size = 0;
			int id = in.getEntry(size);
			if(id == VNL_IO_CALL) {
				uint32_t hash;
				in.getHash(hash);
				bool res = vni_call(in, hash, size);
				if(!res) {
					in.skip(id, size, hash);
					log(ERROR).out << "VNI_CALL failed: hash=" << hash << ", num_args=" << size << vnl::endl;
				}
			} else {
				in.skip(id, size);
			}
		}
	}
	
private:
	vnl::io::File file;
	vnl::io::TypeOutput out;
	
};


} // vnl

#endif /* INCLUDE_VNL_DATABASE_H_ */
