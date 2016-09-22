/*
 * Recorder.h
 *
 *  Created on: Jul 4, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_RECORDER_H_
#define CPP_INCLUDE_VNI_RECORDER_H_

#include <vnl/RecorderSupport.hxx>
#include <vnl/RecordValue.hxx>
#include <vnl/RecordHeader.hxx>
#include <vnl/Sample.h>
#include <vnl/io/File.h>


namespace vnl {

class Recorder : public vnl::RecorderBase {
public:
	Recorder(const vnl::String& domain_)
		:	RecorderBase(domain_, "Recorder"), out(&file)
	{
	}
	
protected:
	void main() {
		char buf[1024];
		String filepath = filename << vnl::currentDate("%Y%m%d_%H%M%S") << ".dat";
		filepath.to_string(buf, sizeof(buf));
		file = ::fopen(buf, "w");
		if(!file.good()) {
			log(ERROR).out << "Unable to open file for writing: " << filepath << vnl::endl;
			return;
		}
		log(INFO).out << "Recording to file: " << filepath << vnl::endl;
		header.header_size = header_size;
		if(do_write_header) {
			write_header();
			::fseek(file, header_size-4, SEEK_SET);
			out.writeInt(-1);
			out.flush();
		}
		for(String& domain : domains) {
			subscribe(domain, "");
			domain_map[vnl::hash64(domain)] = domain;
			log(DEBUG).out << "Recording domain: " << domain << vnl::endl;
		}
		set_timeout(interval, std::bind(&Recorder::update, this), VNL_TIMER_REPEAT);
		run();
		if(do_write_header) {
			write_header();
		}
		::fflush(file);
		::fclose(file);
	}
	
	bool handle(vnl::Packet* packet) {
		if(packet->pkt_id == vnl::Sample::PID) {
			vnl::Sample* sample = (vnl::Sample*)packet->payload;
			if(sample->data) {
				uint64_t domain = packet->dst_addr.domain();
				if(domain_map.find(domain)) {
					vnl::RecordValue rec;
					rec.time = vnl::currentTimeMicros();
					rec.domain = domain;
					rec.topic = packet->dst_addr.topic();
					rec.value = sample->data;
					vnl::write(out, rec);
					out.flush();
					if(!header.num_samples++) {
						header.begin_time = rec.time;
					}
					header.end_time = rec.time;
					rec.value.release();
				}
			}
		}
		return Super::handle(packet);
	}
	
private:
	void update() {
		if(do_write_header) {
			write_header();
		}
		::fflush(file);
	}
	
	void write_header() {
		::fseek(file, 0, SEEK_SET);
		vnl::write(out, header);
		out.flush();
		::fseek(file, 0, SEEK_END);
	}
	
private:
	vnl::io::File file;
	vnl::io::TypeOutput out;
	
	RecordHeader header;
	
	vnl::Map<uint64_t, String> domain_map;
	
};



}

#endif /* CPP_INCLUDE_VNI_RECORDER_H_ */
