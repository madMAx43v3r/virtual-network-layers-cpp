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
#include <vnl/IOException.hxx>


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
		read_all();
		log(INFO).out << "Finished reading database." << vnl::endl;
		run();
		::fflush(file);
		::fclose(file);
	}
	
	bool handle(Frame* frame) {
		if(out.error()) {
			return false;
		}
		Frame* result = Super::exec_vni_call(frame);
		if(result) {
			if(frame->type == Frame::CALL && result->type == Frame::RESULT) {
				out.writeBinary(frame->data, frame->size);
				out.flush();
				if(out.error()) {
					log(ERROR).out << "Failed to write, error=" << out.error() << vnl::endl;
				}
			}
			send_async(result, frame->src_addr);
		}
		return false;
	}
	
private:
	void read_all() {
		::fseek(file, 0, SEEK_SET);
		int64_t last_pos = 0;
		vnl::io::TypeInput in(&file);
		while(true) {
			last_pos = ::ftell(file);
			// TODO: account for data in buffer
			int size = 0;
			int id = in.getEntry(size);
			if(in.error()) {
				::fseek(file, 0, SEEK_END);
				break;
			}
			if(id == VNL_IO_CALL) {
				uint32_t hash;
				in.getHash(hash);
				try {
					bool res = vni_call(in, hash, size);
					if(in.error()) {
						log(ERROR).out << "Unexpected read error at position " << last_pos << vnl::endl;
						if(ignore_errors) {
							::fseek(file, last_pos, SEEK_SET);
							break;
						} else {
							throw vnl::IOException();
						}
					}
					if(!res) {
						in.skip(id, size, hash);
						log(ERROR).out << "VNI_CALL failed: hash=" << hash << ", num_args=" << size << vnl::endl;
					}
				} catch (const vnl::Exception& ex) {
					if(ignore_errors) {
						log(WARN).out << "Ignored exception: " << ex.get_type_name() << vnl::endl;
					} else {
						ex.raise();
					}
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
