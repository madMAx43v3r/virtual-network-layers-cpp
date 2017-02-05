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
		if(!filename.empty()) {
			open();
			if(file && !readonly) {
				set_timeout(interval, std::bind(&Database::update, this), VNL_TIMER_REPEAT);
			}
		}
		run();
		if(file) {
			::fclose(file);
		}
	}
	
	bool handle(Frame* frame) {
		Frame* result = Super::exec_vni_call(frame);
		if(result) {
			if(file && !readonly && frame->type == Frame::CALL && result->type == Frame::RESULT) {
				out.writeBinary(frame->data, frame->size);
				out.flush();
				if(out.error()) {
					readonly = true;
					log(ERROR).out << "Failed to write, error=" << out.error() << vnl::endl;
				}
			}
			send_async(result, frame->src_addr, true);
		}
		return false;
	}
	
	void update() {
		::fflush(file);
	}
	
private:
	void open() {
		char buf[1024];
		filename.to_string(buf, sizeof(buf));
		if(readonly) {
			file = ::fopen(buf, "rb");
		} else {
			FILE* tmp = ::fopen(buf, "ab+");
			::fclose(tmp);
			file = ::fopen(buf, "rb+");
		}
		if(!file) {
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
		::fseek(file, 0, SEEK_SET);
		int64_t count = 0;
		int64_t last_pos = 0;
		vnl::io::TypeInput in(&file);
		while(true) {
			last_pos = in.get_input_pos();
			int size = 0;
			int id = in.getEntry(size);
			if(id == VNL_IO_CALL) {
				uint32_t hash;
				in.getHash(hash);
				try {
					bool res = vni_call(in, hash, size);
					if(!res) {
						in.skip(id, size, hash);
						log(ERROR).out << "VNI_CALL failed: hash=" << hash << ", num_args=" << size << vnl::endl;
					}
				} catch (const vnl::Exception& ex) {
					if(ignore_errors) {
						log(WARN).out << "Ignored exception: " << ex.get_type_name() << vnl::endl;
					} else {
						::fclose(file);
						ex.raise();
					}
				}
				count++;
			} else {
				in.skip(id, size);
			}
			if(in.error() == VNL_IO_EOF) {
				::fseek(file, 0, SEEK_END);
				break;
			} else if(in.error()) {
				log(ERROR).out << "Read error at position " << last_pos << vnl::endl;
				if(ignore_errors) {
					::fseek(file, last_pos, SEEK_SET);
					break;
				} else {
					throw vnl::IOException();
				}
			}
		}
		log(INFO).out << "Replayed " << count << " transactions, " << in.get_num_read()/1024 << " kB" << vnl::endl;
	}
	
private:
	vnl::io::File file;
	vnl::io::TypeOutput out;
	
};


} // vnl

#endif /* INCLUDE_VNL_DATABASE_H_ */
